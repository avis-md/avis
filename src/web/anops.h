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
#include "anweb.h"
#include "utils/ssh.h"

class AnOps {
public:
	static bool expanded;
	static byte connectStatus;
	static float expandPos;

	static bool remote;
	static std::string user, ip, pw;
	static ushort port;

	static std::string path;
	static std::string message;

	static void Draw();
	static std::thread* conThread;
	static void Connect(), Disconnect();
	static void SendNodes(bool cp);
	static void DoSendNodes(std::string p, std::string rp);
	static void SendIn(), RecvOut();

	static SSH ssh;
};