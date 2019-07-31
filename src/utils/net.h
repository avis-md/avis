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
#include <mutex>

#ifndef PLATFORM_WIN
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define ERROR_SUCCESS 0
#endif

typedef void(*dataReceivedCallback)(uint ip, uint port, byte* data, uint dataCount);

class Net {
public:
	static void Init();
	
	static std::vector<std::string> MyIp();
	
	static bool Listen(uint port, dataReceivedCallback callback);
	static bool StopListen();

	static bool Send(std::string ip, uint port, byte* data, uint dataSz);

	//static std::string IP2String(uint ip);
	//static uint String2IP(const std::string& s);

	static dataReceivedCallback onDataReceived;

protected:
#ifdef PLATFORM_WIN
	static WSADATA* wsa;
	static SOCKET socket;

	static bool InitWsa();
#else
	static int socket;
#endif
	static sockaddr_in me;
	static sockaddr_in other;

	//static std::mutex mutex;
	static std::thread* receivingThread;

	static bool listening;
	static void HostLoop();
};