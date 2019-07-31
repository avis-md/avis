// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "Engine.h"
#include "particles.h"

class ParMenu {
public:
	static int activeMenu;
	static int activeSubMenu[5];
	static std::string menuNames[5];
	static bool expanded;
	static float expandPos;
	static bool showSplash;

	static uint selCnt, listH;
	static float listHOff;
	static byte drawTypeAll, _drawTypeAll;
	static bool visibleAll;

	static std::vector<std::string> recentFiles, recentFilesN;

	static void Init();
	static void CalcH();
	static void Draw(), Draw_List(float off), DrawSplash();
	static void DrawBg(), DrawLoading();
	static void DrawConnMenu(Particles::conninfo& info, float x, float& y, float w);
	static void SelClear(), SelInv(), SelAll();
	static void DrawSelPopup();

	static void LoadRecents(), SaveRecents(const std::string& entry), RemoveRecent(uint i), WriteRecents();
};