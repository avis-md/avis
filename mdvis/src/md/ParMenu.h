#pragma once
#include "Engine.h"
#include "Particles.h"

class ParMenu {
public:

	static int activeMenu;
	static const string menuNames[5];
	static bool expanded;
	static float expandPos;

	static uint selCnt;
	static byte drawTypeAll, _drawTypeAll;

	static void Draw(), Draw_List(), Draw_Vis();
	static void SelClear(), SelInv(), SelAll();
};