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