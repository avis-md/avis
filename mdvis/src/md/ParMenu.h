#pragma once
#include "Engine.h"
#include "Particles.h"

class ParMenu {
public:
	static int activeMenu;
	static int activeSubMenu[5];
	static std::string menuNames[5];
	static bool expanded;
	static float expandPos;
	static bool showSplash;

	static uint selCnt;
	static byte drawTypeAll, _drawTypeAll;
	static bool visibleAll;

	static std::vector<std::string> recentFiles, recentFilesN;

	static void Init();
	static void Draw(), Draw_List(float off), DrawStart(), DrawSplash();
	static void DrawConnMenu(Particles::conninfo& info, float x, float& y, float w);
	static void SelClear(), SelInv(), SelAll();
	static void DrawSelPopup();

	static void LoadRecents(), SaveRecents(const std::string& entry), RemoveRecent(uint i), WriteRecents();
};