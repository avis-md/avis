#pragma once
#include "Engine.h"
#ifdef PLATFORM_WIN
#define ssize_t _W64 int
#endif
#include <libssh2.h>
#include <libssh2_sftp.h>
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
	bool ok;

	static void Init();
	static SSH Connect(const SSHConfig& conf);
	string Read(uint maxlen);
	bool Write(string s);
	void Flush();
	bool WaitFor(string s, uint rate, uint timeout = -1);
	void EnableSFTP(), DisableSFTP();
	void MkDir(const string& path);
	std::vector<string> ListFiles(const string& path);
	bool HasFile(const string& path);
	void GetFile(const string& from, const string& to), SendFile(const string& from, const string& to);
	bool HasCmd(string cmd, string& path);
	void Disconnect();
	void EnableDump(uint rate);

	int sock;
	LIBSSH2_SESSION* session;
	LIBSSH2_CHANNEL* channel;
	LIBSSH2_SFTP* sftpChannel;

	bool stopDump;
	std::thread* dumpthread;
	static void DoDump(SSH* inst, uint rate);
};