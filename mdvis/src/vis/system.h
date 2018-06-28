#pragma once
#include "Engine.h"

enum class VIS_MOUSE_MODE : byte {
	ROTATE,
	PAN,
	SELECT
};

class VisSystem {
public:
	static Vec4 accentColor;
	static uint renderMs, uiMs;

	static VIS_MOUSE_MODE mouseMode;
	
	static float _defBondLength;
	static std::unordered_map<uint, float> _bondLengths;
	static std::unordered_map<ushort, Vec3> _type2Col;
	static std::unordered_map<ushort, std::array<float, 2>> radii;

	static void Init();

	static bool InMainWin(const Vec2& pos);

	static void DrawBar();
};