#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ssh.h"
#include <libssh2_sftp.h>
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
		Debug::Error("SSH", "connect to port fail!");
		return ssh;
	}

	ssh.session = libssh2_session_init();
	if (!!libssh2_session_handshake(ssh.session, ssh.sock)) {
		Debug::Error("SSH", "session fail!");
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
			Debug::Error("SSH", "auth via public key fail!");
			return ssh;
		}
		break;
	case SSH_Auth::PASSWORD:
		if (!!libssh2_userauth_password(ssh.session, &conf.user[0], &conf.pw[0])) {
			Debug::Error("SSH", "auth via password fail!");
			return ssh;
		}
		break;
	}
	if (!(ssh.channel = libssh2_channel_open_session(ssh.session))) {
		Debug::Error("SSH", "open session fail!");
		return ssh;
	}
	if (!!libssh2_channel_request_pty(ssh.channel, "vanilla")) {
		Debug::Error("SSH", "request pty fail!");
		return ssh;
	}
	if (!!libssh2_channel_shell(ssh.channel)) {
		Debug::Error("SSH", "request shell on pty fail!");
		return ssh;
	}
	Debug::Message("SSH", "Connected to " + conf.user + "@" + conf.ip + ":" + std::to_string(conf.port) + ".");
	ssh.ok = true;
	return ssh;
}

string SSH::Read(uint maxlen) {
	char* c = new char[maxlen];
	auto sz = libssh2_channel_read(channel, c, maxlen);
	string s(c, sz);
	delete[](c);
	return s;
}

bool SSH::Write(string s) {
	s += "\n";
	auto sz = libssh2_channel_write(channel, &s[0], s.size());
	return sz == s.size();
}

void SSH::EnableDump(uint rate) {
	stopDump = false;
	dumpthread = new std::thread(DoDump, *this, rate);
}

void SSH::DoDump(SSH& inst, uint rate) {
	while (!inst.stopDump) {
		auto s = inst.Read(1000);
		if (s.size()) std::cout << s;
		Sleep(rate);
	}
}