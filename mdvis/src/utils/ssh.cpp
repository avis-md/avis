#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ssh.h"
#include "utils/net.h"

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
	
	ssh.sock = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(conf.port);
	sin.sin_addr.s_addr = inet_addr(&conf.ip[0]);
	if (!!connect(ssh.sock, (struct sockaddr*)&sin, sizeof(sin))) {
		Debug::Warning("SSH", "connect to port fail!");
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
	if (!!libssh2_channel_request_pty(ssh.channel, "vanilla")) {
		Debug::Warning("SSH", "request pty fail!");
		return ssh;
	}
	if (!!libssh2_channel_shell(ssh.channel)) {
		Debug::Warning("SSH", "request shell on pty fail!");
		return ssh;
	}
	Debug::Message("SSH", "Connected to " + conf.user + "@" + conf.ip + ":" + std::to_string(conf.port) + ".");
	ssh.ok = true;
	return ssh;
}

void SSH::Disconnect() {
	if (channel) libssh2_channel_free(channel);
	libssh2_session_disconnect(session, "logout");
	libssh2_session_free(session);
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

void SSH::Flush() {
	Write("echo '#''#'");
	WaitFor("##", 200);
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

std::vector<std::string> SSH::ListFiles(const std::string& path) {
	std::vector<std::string> res;
	auto hnd = libssh2_sftp_opendir(sftpChannel, path.c_str());
	if (hnd) {
		for (;;) {
			char mem[512], lentry[512];
			LIBSSH2_SFTP_ATTRIBUTES attr;
			auto rc = libssh2_sftp_readdir_ex(hnd, mem, 512, lentry, 512, &attr);
			if (rc > 0) {
				res.push_back(std::string(mem, rc));
			}
			else break;
		}
	}
	return res;
}

bool SSH::HasFile(const std::string& path) {
	auto cmd = "echo '!''<'; test -e " + path + " && echo 'file found!!''>' || echo '!''>'";
	Write(cmd);
	std::string s;
	for (;;) {
		s += Read(500);
		Engine::Sleep(100);
		auto pos0 = string_find(s, "!<");
		auto pos1 = string_find(s, "!>");
		if (pos0 == -1) s = "";
		if (pos1 == -1) continue;
		if (pos1 > (pos0 + 5)) {
			return true;
		}
		else {
			return false;
		}
	}
}

void SSH::GetFile(const std::string& from, const std::string& to) {
	Flush();
	auto hnd = libssh2_sftp_open(sftpChannel, &from[0], LIBSSH2_FXF_READ, 0);
	std::ofstream strm(to, std::ios::binary);
	if (!hnd || !strm) {
		return;
	}
	else {
		char mem[4096];
		for (;;) {
			auto wc = libssh2_sftp_read(hnd, mem, 4096);
			if (wc <= 0) break;
			strm.write(mem, wc);
		}
		libssh2_sftp_close(hnd);
		strm.close();
	}
}

void SSH::MkDir(const std::string& path) {
	libssh2_sftp_mkdir(sftpChannel, &path[0], LIBSSH2_SFTP_S_IRWXU |
		LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IXGRP |
		LIBSSH2_SFTP_S_IROTH | LIBSSH2_SFTP_S_IXOTH);
}

void SSH::SendFile(const std::string& from, const std::string& to) {
	Flush();
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