#pragma once
#include "Engine.h"
#include "Particles.h"

class ParMenu {
public:

	static int activeMenu;
	static const string menuNames[4];
	static bool expanded;
	static float expandPos;

	static void Draw(), Draw_List(), Draw_Vis();
};