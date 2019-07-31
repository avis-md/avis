// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ssh.h"
#include "utils/net.h"
#ifdef PLATFORM_WIN
#include <WS2tcpip.h>
#else
#include <netdb.h>
#endif

#define SFTP_BUF_SZ 65535

SSH::SSH() : ok(false), session(nullptr) {}

SSH::~SSH() {
	CheckUniqueRef();
}

void SSH::Init() {
	int res = libssh2_init(0);
	if (!!res) {
		Debug::Error("SSH", "Failed to init! Err code: " + std::to_string(res));
	}
	Net::Init();
	Debug::Message("SSH", "Init finished.");
	Unloader::Reg(Deinit);
}

void SSH::Deinit() {
	Debug::Message("SSH", "Exit");
	libssh2_exit();
}

SSH SSH::Connect(const SSHConfig& conf) {
	SSH ssh;
	Debug::Message("SSH", "Starting session...");
	
	struct sockaddr* sin;
	socklen_t ssz = sizeof (struct sockaddr);
	
	if (conf.ip[0] > '0') {
		struct addrinfo hints = {}, *result;
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = 0;
		hints.ai_canonname = nullptr;
		hints.ai_addr = nullptr;
		hints.ai_next = nullptr;
		if (!!getaddrinfo(&conf.ip[0], &std::to_string(conf.port)[0], &hints, &result)) {
			Debug::Warning("SSH", "resolve address fail!");
			return ssh;
		}
		sin = result->ai_addr;
		ssz = result->ai_addrlen;
		//conf.ip = std::string(result->ai_addr, result->ai_addrlen);
	}
	else {
		static sockaddr_in sn;
		sn.sin_family = AF_INET;
		sn.sin_port = htons(conf.port);
		sn.sin_addr.s_addr = inet_addr(&conf.ip[0]);
		sin = (struct sockaddr*)&sn;
	}
	ssh.sock = socket(AF_INET, SOCK_STREAM, 0);
	if (!!connect(ssh.sock, sin, ssz)) {
		Debug::Warning("SSH", "connect to host fail!");
		return ssh;
	}

	ssh.session = libssh2_session_init();
	if (!!libssh2_session_handshake(ssh.session, ssh.sock)) {
		Debug::Warning("SSH", "session fail!");
		return ssh;
	}
	auto fp = libssh2_hostkey_hash(ssh.session, LIBSSH2_HOSTKEY_HASH_SHA1);
	std::stringstream ss;
	for (int i = 0; i < 20; i++)
		ss << std::hex << (int)fp[i];
	Debug::Message("SSH", "Host fingerprint is " + ss.str());
	Debug::Message("SSH", "Authenticating...");
	switch (conf.auth) {
	case SSH_Auth::PUBKEY:
		if (!!libssh2_userauth_publickey_fromfile(ssh.session, &conf.user[0], &conf.keyPath1[0], &conf.keyPath2[0], &conf.pw[0])) {
			Debug::Warning("SSH", "auth via public key fail!");
			return ssh;
		}
		break;
	case SSH_Auth::PASSWORD:
		if (!!libssh2_userauth_password(ssh.session, &conf.user[0], &conf.pw[0])) {
			Debug::Warning("SSH", "auth via password fail!");
			return ssh;
		}
		break;
	}
	if (!(ssh.channel = libssh2_channel_open_session(ssh.session))) {
		Debug::Warning("SSH", "open session fail!");
		return ssh;
	}
	libssh2_session_set_blocking(ssh.session, 1);
	if (!!libssh2_channel_request_pty(ssh.channel, "vanilla")) {
		Debug::Warning("SSH", "request pty fail!");
		return ssh;
	}
	if (!!libssh2_channel_shell(ssh.channel)) {
		Debug::Warning("SSH", "request shell on pty fail!");
		return ssh;
	}
	Debug::Message("SSH", "Connected to " + conf.user + "@" + conf.ip + ":" + std::to_string(conf.port) + ".");
	
	ssh.EnableSFTP();
	ssh.GetUserPath();
	ssh.ok = true;
	return ssh;
}

void SSH::Disconnect() {
	if (session) {
		Debug::Message("SSH", "Killing session...");
		DisableSFTP();
		if (channel) libssh2_channel_close(channel);
		libssh2_session_disconnect(session, "Bye bye");
		libssh2_session_free(session);
	}
	session = nullptr;
	ok = false;
}

void SSH::GetUserPath() {
	const std::string tpf = "/tmp/avis_userpath.txt";
	Write("rm " + tpf + ";cd && pwd > " + tpf);
	auto res = GetFile(tpf);
	userPath = std::string(&res[0], res.size());
	while (userPath.back() == '\n') userPath.pop_back();
	Debug::Message("SSH", "User path is \"" + userPath + "\"");
}

std::string SSH::ResolveUserPath(const std::string& path) {
	if (path[0] == '~') {
		if (path[1] == '/')
			return userPath + path.substr(1);
	}
	return path;
}

std::string SSH::Read(uint maxlen) {
	std::vector<char> c(maxlen);
	auto sz = libssh2_channel_read(channel, c.data(), maxlen);
	std::string s(c.data(), sz);
	return s;
}

bool SSH::Write(std::string s) {
	Debug::Message("SSH::Write", s);
	s += "\n";
	auto sz = s.size();
	int off = 0;
	size_t tot = 0;
	do {
		auto res = libssh2_channel_write(channel, &s[off], s.size());
		if (res != LIBSSH2_ERROR_EAGAIN) {
			if (res < 0) return false;
			off += res;
			tot += res;
		}
	} while (tot != sz);
	return true;
}

bool SSH::WaitFor(std::string s, uint rate, uint timeout) {
	std::string o = "";
	uint t = 0;
	auto ms = s.size();
	while(t < timeout) {
		o += Read(500);
		if (string_find(o, s) > -1) return true;
		o = o.substr(0, o.size() - ms);
		Engine::Sleep(rate);
		t += rate;
	}
	return false;
}

void SSH::EnableDump(uint rate) {
	stopDump = false;
	dumpthread = new std::thread(DoDump, this, rate);
}

void SSH::DoDump(SSH* inst, uint rate) {
	while (!inst->stopDump) {
		auto s = inst->Read(1000);
		if (s.size()) std::cout << s;
		Engine::Sleep(100);
	}
}

void SSH::EnableSFTP() {
	sftpChannel = libssh2_sftp_init(session);
}

void SSH::DisableSFTP() {
	if (sftpChannel) {
		libssh2_sftp_shutdown(sftpChannel);
		sftpChannel = 0;
	}
}

//list files and folders. Please reimplement this with a more elegant solution if possible.
#define TD "/tmp/avis_testdir"
void SSH::ListFD(std::string path, std::vector<std::string>& fls, std::vector<std::string>& fds) {
	fls.clear();
	fds.clear();
	path = ResolveUserPath(path);
	std::vector<std::string> res;
	libssh2_sftp_unlink(sftpChannel, TD);
	libssh2_sftp_unlink(sftpChannel, TD "__");
	Write("ls -pA " + path + " > " TD + "; echo 1 > " TD "__");
	auto cc = GetFile(TD "__", 50, 100);
	if (!cc.size()) {
		Debug::Warning("SSH::ListFD", "cannot read temp path file!");
		return;
	}
	cc = GetFile(TD);
	auto ss = string_split(std::string(cc.data(), cc.size()), '\n', true);
	for (auto& s : ss) {
		if (!s.size()) continue;
		if (s.back() == '/') {
			s.pop_back();
			fds.push_back(s);
		}
		else
			fls.push_back(s);
	}
}

std::vector<std::string> SSH::ListFiles(std::string path) {
	std::vector<std::string> fls, fds;
	ListFD(path, fls, fds);
	return fls;
}

std::vector<std::string> SSH::ListDirs(std::string path) {
	std::vector<std::string> fls, fds;
	ListFD(path, fls, fds);
	return fds;
}

bool SSH::HasFile(std::string path) {
	path = ResolveUserPath(path);
	LIBSSH2_SFTP_ATTRIBUTES info = {};
	auto res = libssh2_sftp_stat(sftpChannel, path.c_str(), &info);
	return !res;
}

std::vector<char> SSH::GetFile(std::string from, int tries, uint period) {
	Debug::Message("SSH", "Get \"" + from + "\"");
	from = ResolveUserPath(from);
	LIBSSH2_SFTP_HANDLE* hnd = nullptr;
	std::vector<char> res;
	for (int a = 0; a < tries && !hnd; ++a) {
		hnd = libssh2_sftp_open(sftpChannel, &from[0], LIBSSH2_FXF_READ, 0);
		if (hnd) break;
		if (a == tries - 1) {
			Debug::Warning("SSH::GetFile", "cannot open file!");
			return res;
		}
		Engine::Sleep(period);
	}
	size_t sz = 0;
	auto tm = milliseconds();
	std::array<char, SFTP_BUF_SZ> mem;
	for (;;) {
		auto wc = libssh2_sftp_read(hnd, &mem[0], SFTP_BUF_SZ);
		if (wc <= 0) {
			if (wc < 0) Debug::Warning("SSH::GetFile", "read returns " + std::to_string(wc) + "!");
			break;
		}
		res.resize(sz + wc);
		memcpy(&res[sz], &mem[0], wc);
		sz += wc;
	}
	libssh2_sftp_close(hnd);
	tm = milliseconds() - tm;
	Debug::Message("SSH", "Got " + std::to_string(sz) + " in " + std::to_string(tm * 0.001f) + " (" + std::to_string((sz * 1000) / tm) + "B/s)");
	return res;
}

void SSH::GetFile(std::string from, std::string to) {
	if (!HasFile(from)) return;
	from = ResolveUserPath(from);
	to = IO::ResolveUserPath(to);
	auto res = GetFile(from);
	if (!res.size()) return;
	std::ofstream strm(to, std::ios::binary);
	if (strm) {
		strm.write(&res[0], res.size());
	}
}

bool SSH::MkDir(const std::string& path) {
	auto ret = libssh2_sftp_mkdir(sftpChannel, &path[0], LIBSSH2_SFTP_S_IRWXU |
		LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IXGRP |
		LIBSSH2_SFTP_S_IROTH | LIBSSH2_SFTP_S_IXOTH);
	return !ret;
}

void SSH::SendFile(std::string from, std::string to) {
	from = IO::ResolveUserPath(from);
	to = ResolveUserPath(to);
	std::ifstream strm(from, std::ios::binary); 
	auto hnd = libssh2_sftp_open(sftpChannel, &to[0], 
		LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC | 
		LIBSSH2_SFTP_S_IRWXU | LIBSSH2_SFTP_S_IRWXG | LIBSSH2_SFTP_S_IROTH, 0);
	if (!hnd || !strm) {
		char* err;
		libssh2_session_last_error(session, &err, 0, 0);
		Debug::Warning("SSH", "Failed to send file: " + std::string(err));
		return;
	}
	else {
		char mem[4096];
		std::streamsize cnt = 0;
		for (;;) {
			strm.read(mem, 4096);
			cnt = strm.gcount();
			if (!cnt)
				break;
			uint loc = 0;
			while (cnt > 0) {
				auto rc = libssh2_sftp_write(hnd, mem + loc, (size_t)cnt);
				cnt -= rc;
				loc += rc;
			}
		}
		libssh2_sftp_close(hnd);
		strm.close();
	}
}

bool SSH::HasCmd(std::string cmd, std::string& path) {
	cmd = "echo \"<\"\"<\"; command -v " + cmd + "; echo \">\"\">\"";
	Write(cmd);
	std::string s;
	for (;;) {
		s += Read(100);
		Engine::Sleep(100);
		auto pos0 = string_find(s, "<<");
		auto pos1 = string_find(s, ">>");
		if (pos0 == -1 || pos1 == -1) continue;
		if (pos1 > (pos0 + 5)) {
			path = s.substr(pos0 + 3, pos1 - pos0 - 4);
			return true;
		}
		else {
			path = "";
			return false;
		}
	}
}