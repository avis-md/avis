#pragma once
#include "anweb.h"
#include "utils/ssh.h"

class AnOps {
public:
	static bool expanded;
	static byte connectStatus;
	static float expandPos;

	static bool remote;
	static string user, ip, pw;
	static ushort port;

	static string path;

	static void Draw();
	static std::thread* conThread;
	static void Connect(), Disconnect();

	static SSH ssh;
};