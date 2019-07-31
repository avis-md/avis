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

enum class POPUP_TYPE : byte {
    NONE,
	MENU,
    DRAWMODE,
	COLORPICK,
	DROPDOWN,
	RESNM,
	RESID,
	ATOMID,
	SYSMSG
};

class Popups {
public:
	struct MenuItem {
		typedef void(*CBK)();

		GLuint icon = 0;
		std::string label;
		std::vector<MenuItem> child;
		CBK callback;

		void Set(const Texture& tex, const std::string& str, CBK cb) {
			icon = tex ? tex.pointer : 0;
			label = str;
			callback = cb;
		}
	};
	struct DropdownItem {
		DropdownItem(uint* a = 0, std::string* b = 0, bool f = false) : target(a), list(b), flags(f) {}

		uint* target;
		std::string* list;
		bool seld, flags;
	};

    static POPUP_TYPE type;
    static Vec2 pos, pos2;
    static void* data;
	static int selectedMenu;

    static void Draw(), DrawMenu(), DrawDropdown();
	static bool DoDrawMenu(std::vector<MenuItem>* mn, float x, float y, size_t* act);
};