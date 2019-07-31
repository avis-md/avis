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

#pragma once
#include "Engine.h"
#ifdef PLATFORM_WIN
#define ssize_t _W64 int
#endif
#include <libssh2.h>
#include <libssh2_sftp.h>
#undef ssize_t
#include <unordered_set>

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

class SSH : public RefCnt {
public:
	bool ok;

	static void Init(), Deinit();
	static SSH Connect(const SSHConfig& conf);

	SSH();
	~SSH();

	void GetUserPath();
	std::string ResolveUserPath(const std::string& s);
	std::string Read(uint maxlen);
	bool Write(std::string s);
	bool WaitFor(std::string s, uint rate, uint timeout = -1);
	void EnableSFTP(), DisableSFTP();
	bool MkDir(const std::string& path);
	void ListFD(std::string path, std::vector<std::string>& fls, std::vector<std::string>& fds);
	std::vector<std::string> ListFiles(std::string path);
	std::vector<std::string> ListDirs(std::string path);
	bool HasFile(std::string path);
	std::vector<char> GetFile(std::string from, int tries = 1, uint period = 100);
	void GetFile(std::string from, std::string to), SendFile(std::string from, std::string to);
	bool HasCmd(std::string cmd, std::string& path);
	void DestroyRef() override { Disconnect(); }
	void Disconnect();
	void EnableDump(uint rate);

	int sock;
	LIBSSH2_SESSION* session;
	LIBSSH2_CHANNEL* channel;
	LIBSSH2_SFTP* sftpChannel;

	std::string userPath;

	bool stopDump;
	std::thread* dumpthread;
	static void DoDump(SSH* inst, uint rate);
};