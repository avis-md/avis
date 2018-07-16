#pragma once
#include "Engine.h"
#include "Particles.h"

class ParMenu {
public:

	static int activeMenu;
	static int activeSubMenu[5];
	static const string menuNames[5];
	static bool expanded;
	static float expandPos;

	static uint selCnt;
	static byte drawTypeAll, _drawTypeAll;
	static bool visibleAll;

	static void Draw(), Draw_List(float off);
	static void SelClear(), SelInv(), SelAll();
};