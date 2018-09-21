#pragma once
#include "Engine.h"
#ifdef PLATFORM_WIN
#define ssize_t _W64 int
#endif
#include <libssh2.h>
#include <libssh2_sftp.h>

enum class SSH_Auth {
	PUBKEY,
	PASSWORD
};

struct SSHConfig {
	std::string ip;
	ushort port = 22;
	std::string user;
	SSH_Auth auth = SSH_Auth::PUBKEY;
	std::string keyPath1, keyPath2;
	std::string pw;
};

class SSH {
public:
	bool ok;

	static void Init();
	static SSH Connect(const SSHConfig& conf);
	std::string Read(uint maxlen);
	bool Write(std::string s);
	void Flush();
	bool WaitFor(std::string s, uint rate, uint timeout = -1);
	void EnableSFTP(), DisableSFTP();
	void MkDir(const std::string& path);
	std::vector<std::string> ListFiles(const std::string& path);
	bool HasFile(const std::string& path);
	void GetFile(const std::string& from, const std::string& to), SendFile(const std::string& from, const std::string& to);
	bool HasCmd(std::string cmd, std::string& path);
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