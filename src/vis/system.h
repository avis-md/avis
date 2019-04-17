#pragma once
#include "Engine.h"
#include "ui/popups.h"

#define EXT_SVFL ".zip"

enum class VIS_MOUSE_MODE : byte {
	ROTATE,
	PAN,
	SELECT
};

class VisSystem {
public:
	static std::string version_hash;

	static std::string localFd;

	static Vec4 accentColor, backColor;
	static bool blur;
	static float blurRad;
	static float opacity;
	static uint renderMs, uiMs;

	static float lastSave;
	static std::string currentSavePath, currentSavePath2;

	static std::vector<Popups::MenuItem> menuItems[10];

	static std::string message, message2;
	static bool hasMessage2;
	static byte messageSev;

	static void SetMsg(const std::string& msg, byte sev = 0, const std::string& msg2 = "");

	static VIS_MOUSE_MODE mouseMode;
	
	static float _defBondLength;
	static std::unordered_map<uint, float> _bondLengths;
	static std::unordered_map<ushort, float> radii;

	static void Init(), InitEnv();

	static bool InMainWin(const Vec2& pos);

	static void UpdateTitle();
	static void DrawTitle(), DrawBar(), DrawMsgPopup();

	static void BlurBack();

	static void Save(const std::string& path);
	static bool Load(const std::string& path);

	static void RegisterPath();
};