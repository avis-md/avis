#pragma once
#include "Engine.h"
#ifdef PLATFORM_WIN
#define ssize_t _W64 int
#endif
#include <libssh2.h>
#ifdef PLATFORM_WIN
#pragma comment(lib, "libssh2_win.lib")
#endif

enum class SSH_Auth {
	PUBKEY,
	PASSWORD
};

struct SSHConfig {
	string ip;
	ushort port = 22;
	string user;
	SSH_Auth auth = SSH_Auth::PUBKEY;
	string keyPath1, keyPath2;
	string pw;
};

class SSH {
public:
	static void Init();
	static SSH Connect(const SSHConfig& conf);
	string Read(uint maxlen);
	bool Write(string s);
	void Disconnect();
	void EnableDump(uint rate);
protected:
	bool ok;
	int sock;
	LIBSSH2_SESSION* session;
	LIBSSH2_CHANNEL* channel;

	bool stopDump;
	std::thread* dumpthread;
	static void DoDump(SSH* inst, uint rate);
};