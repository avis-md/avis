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

#include "Engine.h"
#ifdef PLATFORM_WIN
#include <Ws2tcpip.h>
#include <IPHlpApi.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include "ifaddrs.h"
#endif

#ifdef PLATFORM_WIN
WSADATA* Net::wsa = 0;
SOCKET Net::socket = SOCKET();
#else
int Net::socket = 0;
#endif
sockaddr_in Net::me = sockaddr_in();
sockaddr_in Net::other = sockaddr_in();
//std::mutex Net::mutex;
std::thread* Net::receivingThread = 0;
bool Net::listening = false;

dataReceivedCallback Net::onDataReceived = 0;

void Net::Init() {
#ifdef PLATFORM_WIN
	if (!wsa) InitWsa();
#endif
}

std::vector<std::string> Net::MyIp() {
	std::vector<std::string> res;
	ulong sz = 25000;
	char cbuf[255];
#ifdef PLATFORM_WIN
	IP_ADAPTER_ADDRESSES* buf = (IP_ADAPTER_ADDRESSES*)malloc(sz);
	if (GetAdaptersAddresses(AF_INET, 0, NULL, buf, &sz) == ERROR_SUCCESS) {
		for (; buf; buf = buf->Next) {
			auto unicast = buf->FirstUnicastAddress;
			for (; unicast; unicast = unicast->Next) {
				if (unicast->Address.lpSockaddr->sa_family == AF_INET)
				{
					sockaddr_in *sa_in = (sockaddr_in *)unicast->Address.lpSockaddr;
					inet_ntop(AF_INET, &(sa_in->sin_addr), cbuf, 255);
					res.push_back(std::string(cbuf));
				}
			}
		}
	}
	free(buf);
#else
	ifaddrs* buf = (ifaddrs*)malloc(sz);
	if (getifaddrs(&buf) == ERROR_SUCCESS) {
		for (; buf; buf = buf->ifa_next) {
			if (!buf->ifa_addr) continue;
			if (buf->ifa_addr->sa_family == AF_INET) {
				sockaddr_in *sa_in = (sockaddr_in *)buf->ifa_addr;
				inet_ntop(AF_INET, &(sa_in->sin_addr), cbuf, 255);
				res.push_back(std::string(cbuf));
			}
		}
	}
#endif
	return res;
}

bool Net::Listen(uint port, dataReceivedCallback callback) {
	if (listening) return false;
#ifdef PLATFORM_WIN
	if (!wsa && !InitWsa()) return false;
#endif

	if ((socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
		return false;
	}

	me.sin_family = AF_INET;
	me.sin_addr.s_addr = htonl(INADDR_ANY);//inet_pton(AF_INET, "127.0.0.1", &me.sin_addr.S_un.S_addr);
	me.sin_port = htons(port);
	if (bind(socket, (sockaddr*)&me, sizeof(me)) == SOCKET_ERROR) {
		return false;
	}

	onDataReceived = callback;

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100;
	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

	listening = true;
	receivingThread = new std::thread(HostLoop);
	return true;
}

bool Net::StopListen() {
	listening = false;
	receivingThread->join();
	delete(receivingThread);
	return true;
}

bool Net::Send(std::string ip, uint port, byte* data, uint dataSz) {
#ifdef PLATFORM_WIN
	if (!wsa && !InitWsa()) return false;
#endif

	if ((socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
		return false;
	}

	other.sin_family = AF_INET;
	inet_pton(AF_INET, ip.c_str(), &other.sin_addr.s_addr);
	other.sin_port = htons(port);

	return (sendto(socket, (char*)data, dataSz, 0, (sockaddr*)&other, sizeof(other)) != SOCKET_ERROR);
}

#ifdef PLATFORM_WIN
bool Net::InitWsa() {
	wsa = new WSADATA();
	return !WSAStartup(MAKEWORD(2, 2), wsa);
}
#endif

void Net::HostLoop() {
	byte buffer[256];
	uint sz;
	socklen_t ss = sizeof(sockaddr);
	while (listening) {
		//memset(buffer, 0, sz);
		if ((sz = recvfrom(socket, (char*)buffer, 255, 0, (sockaddr*)&other, &ss)) == SOCKET_ERROR) {
			auto err = errno;
#ifndef PLATFORM_WIN
			if (err == EAGAIN || err == EWOULDBLOCK) //is it just a timeout
				continue;
			else
#endif
			{
				Debug::Warning("Net", "recvfrom failed: " + std::to_string(err));
				return;
			}
		}
		if (!!sz && onDataReceived) {
			onDataReceived(other.sin_addr.s_addr, other.sin_port, buffer, sz);
		}
	}
}
