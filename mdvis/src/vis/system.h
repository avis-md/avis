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
	static Font* font;

	static VIS_MOUSE_MODE mouseMode;
	
	static bool InMainWin(const Vec2& pos);

	static void DrawBar();
};