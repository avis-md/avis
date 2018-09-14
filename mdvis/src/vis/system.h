#pragma once
#include "Engine.h"
#include "ui/popups.h"

enum class VIS_MOUSE_MODE : byte {
	ROTATE,
	PAN,
	SELECT
};

class VisSystem {
public:
	static string version_hash;

	static Vec4 accentColor;
	static float glass;
	static uint renderMs, uiMs;

	static std::vector<MenuItem> menuItems[10];

	static string message, message2;
	static bool hasMessage2;
	static byte messageSev;

	static void SetMsg(const string& msg, byte sev = 0, const string& msg2 = "");

	static VIS_MOUSE_MODE mouseMode;
	
	static float _defBondLength;
	static std::unordered_map<uint, float> _bondLengths;
	static std::unordered_map<ushort, Vec3> _type2Col;
	static std::unordered_map<ushort, std::array<float, 2>> radii;

	static std::unordered_map<string, string> envs, prefs;

	static void Init(), InitEnv();

	static bool InMainWin(const Vec2& pos);

	static void DrawTitle(), DrawBar(), DrawMsgPopup();
};