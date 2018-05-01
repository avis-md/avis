#pragma once
#include "Engine.h"

class ParMenu {
public:

	static int activeMenu;
	static const string menuNames[4];
	static bool expanded;
	static float expandPos;

	static Font* font;

	static void Draw();
};