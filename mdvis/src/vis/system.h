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
	static Vec4 accentColor;
	static uint renderMs, uiMs;

	static std::vector<MenuItem> menuItems[4];

	static string message;

	static VIS_MOUSE_MODE mouseMode;
	
	static float _defBondLength;
	static std::unordered_map<uint, float> _bondLengths;
	static std::unordered_map<ushort, Vec3> _type2Col;
	static std::unordered_map<ushort, std::array<float, 2>> radii;

	static void Init();

	static bool InMainWin(const Vec2& pos);

	static void DrawTitle(), DrawBar();
};