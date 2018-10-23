#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ssh.h"
#include "utils/net.h"
#ifdef PLATFORM_WIN
#include <WS2tcpip.h>
#else
#include <netdb.h>
#endif

#define SFTP_BUF_SZ 65535

void SSH::Init() {
	int res = libssh2_init(0);
	if (!!res) {
		Debug::Error("SSH", "Failed to init! Err code: " + std::to_string(res));
	}
	Net::Init();
	Debug::Message("SSH", "Init finished.");
}

SSH SSH::Connect(const SSHConfig& conf) {
	SSH ssh;
	ssh.ok = false;
	Debug::Message("SSH", "Starting session...");
	
	struct sockaddr* sin;
	socklen_t ssz;
	
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
	if (channel) libssh2_channel_free(channel);
	libssh2_session_disconnect(session, "logout");
	libssh2_session_free(session);
	ok = false;
}

void SSH::GetUserPath() {
	std::string tpf = "/tmp/avis_userpath.txt";
	Write("cd && pwd > " + tpf);
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
	char* c = new char[maxlen];
	auto sz = libssh2_channel_read(channel, c, maxlen);
	std::string s(c, sz);
	delete[](c);
	return s;
}

bool SSH::Write(std::string s) {
	s += "\n";
	auto sz = libssh2_channel_write(channel, &s[0], s.size());
	return sz == s.size();
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

std::vector<std::string> SSH::ListFiles(std::string path) {
	path = ResolveUserPath(path);
	std::vector<std::string> res;
	auto hnd = libssh2_sftp_opendir(sftpChannel, path.c_str());
	if (hnd) {
		for (;;) {
			char mem[512], lentry[512];
			LIBSSH2_SFTP_ATTRIBUTES attr;
			auto rc = libssh2_sftp_readdir_ex(hnd, mem, 512, lentry, 512, &attr);
			if (rc > 0) {
				std::string s(mem, rc);
				if (s != "." && s != "..")
					res.push_back(s);
			}
			else break;
		}
	}
	return res;
}

bool SSH::HasFile(std::string path) {
	path = ResolveUserPath(path);
	LIBSSH2_SFTP_ATTRIBUTES info = {};
	auto res = libssh2_sftp_stat(sftpChannel, path.c_str(), &info);
	return !res;
}

std::vector<char> SSH::GetFile(std::string from) {
	Debug::Message("SSH", "Get \"" + from + "\"");
	from = ResolveUserPath(from);
	auto hnd = libssh2_sftp_open(sftpChannel, &from[0], LIBSSH2_FXF_READ, 0);
	std::vector<char> res;
	auto sz = 0;
	auto tm = milliseconds();
	std::array<char, SFTP_BUF_SZ> mem;
	if (hnd) {
		for (;;) {
			auto wc = libssh2_sftp_read(hnd, &mem[0], SFTP_BUF_SZ);
			if (wc <= 0) break;
			res.resize(sz + wc);
			memcpy(&res[sz], &mem[0], wc);
			sz += wc;
		}
		libssh2_sftp_close(hnd);
	}
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