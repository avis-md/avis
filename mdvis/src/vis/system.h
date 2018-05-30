#pragma once
#include "Engine.h"

enum class VIS_MOUSE_MODE : byte {
	ROTATE,
	PAN,
	SELECT
};

/*
enum class VIS_DRAW_MODE : byte {
	//atoms
	NONE = 0x00,
	BALL = 0x01,
	VDW = 0x02,
	//conns
	LINES = 0x10,
	STICK = 0x20
};
template<typename T> inline VIS_DRAW_MODE operator|(VIS_DRAW_MODE a, T b)
{return static_cast<VIS_DRAW_MODE>(static_cast<uint>(a) | static_cast<uint>(b));}
template<typename T> inline VIS_DRAW_MODE operator&(VIS_DRAW_MODE a, T b)
{return static_cast<VIS_DRAW_MODE>(static_cast<uint>(a) & static_cast<uint>(b));}
*/
class VisSystem {
public:
	static Vec4 accentColor;
	static uint renderMs, uiMs;
	static Font* font;

	static VIS_MOUSE_MODE mouseMode;
	
	static std::unordered_map<uint, float> _bondLengths;
	static std::unordered_map<ushort, Vec3> _type2Col;
	static std::unordered_map<ushort, std::array<float, 2>> radii;

	static void Init();

	static bool InMainWin(const Vec2& pos);

	static void DrawBar();
};