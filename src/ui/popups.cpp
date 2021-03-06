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

#include "popups.h"
#include "vis/pargraphics.h"
#include "md/parmenu.h"
#include "ui/icons.h"

POPUP_TYPE Popups::type = POPUP_TYPE::NONE;
Vec2 Popups::pos, Popups::pos2;
void* Popups::data = 0;
int Popups::selectedMenu;

void Popups::Draw() {
    if (type == POPUP_TYPE::NONE) return;
    UI::IncLayer();

	if (Input::KeyDown(KEY::Escape)) {
		type = POPUP_TYPE::NONE;
		return;
	}

    switch (type) {
	case POPUP_TYPE::MENU:
		DrawMenu();
		break;
    case POPUP_TYPE::DRAWMODE:
        ParGraphics::DrawPopupDM();
        break;
	case POPUP_TYPE::COLORPICK:
		Color::DrawPicker();
		break;
	case POPUP_TYPE::DROPDOWN:
		DrawDropdown();
		break;
	case POPUP_TYPE::RESNM:
	case POPUP_TYPE::RESID:
	case POPUP_TYPE::ATOMID:
		ParMenu::DrawSelPopup();
		break;
	case POPUP_TYPE::SYSMSG:
		VisSystem::DrawMsgPopup();
		break;
    default:
		Debug::Warning("Popup", "case not handled: " + std::to_string((int)type) + "!");
		type = POPUP_TYPE::NONE;
        break;
    }
}

void Popups::DrawMenu() {
	auto mn = (std::vector<MenuItem>*)data;
	static size_t actc[10] = {};
	if (DoDrawMenu(mn, pos.x, pos.y, actc)) {
		type = POPUP_TYPE::NONE;
	}
}

bool Popups::DoDrawMenu(std::vector<MenuItem>* mn, float x, float y, size_t* act) {
	auto sz = mn->size();
	UI::Quad(x - 1, y, 122, 18 * sz + 1.f, black(0.7f));
	UI::Quad(x, y, 120, 18.f * sz, white(1, 0.1f));
	for (size_t a = 0; a < sz; ++a) {
		auto& i = mn->at(a);
		auto st = Engine::Button(x + 1, y + 18 * a + 1, 118, 16, white(0), white(1, 0.2f), white(1, 0.05f));
		if (i.icon != GLuint(-1)) {
			UI::Quad(x + 2, y + 18 * a + 1, 16, 16, i.icon);
			UI::Label(x + 22, y + 18 * a + 1, 12, i.label, white((!st) ? 0.7f : 1));
		}
		else UI::Label(x + 2, y + 18 * a + 1, 12, i.label, white((!st) ? 0.7f : 1));
		if (!!i.child.size()) {
			UI::Texture(x + 119 - 14, y + 18 * a + 3, 12, 12, Icons::right, white(0.2f));
			if ((st & MOUSE_HOVER_FLAG)) {
				*act = a + 1;
			}
		}
		if (st == MOUSE_RELEASE) {
			type = POPUP_TYPE::NONE;
			selectedMenu = a;
			if (i.callback) i.callback();
			return 1;
		}
	}
	if (*act > 0) {
		if (DoDrawMenu(&mn->at(*act-1).child, x + 119, y + 18 * (*act-1) + 1, act+1))
			*act = 0;
	}
	bool clk = (Input::mouse0State == 1) && !Engine::Button(x - 1, y, 122, 18 * sz + 3.f);
	bool hvr = !Rect(x - 30, y - 30, 182, 18 * sz + 63.f).Inside(Input::mousePos);
	return ((clk || hvr) && !*act);
}

void Popups::DrawDropdown() {
	auto dt = (DropdownItem*)data;
	uint n = 0;
	while (!!dt->list[n][0]) n++;
	if (!dt->flags) {
		UI::Quad(pos.x - 1, pos.y, pos2.x + 2, 16 * n + 1.f, black(0.7f));
		for (uint a = 0; a < n; ++a) {
			if (Engine::Button(pos.x, pos.y + 16 * a, pos2.x, 16, white(1, 0.2f), dt->list[a], 12, white()) == MOUSE_RELEASE) {
				(*dt->target) = a;
				Popups::type = POPUP_TYPE::NONE;
				dt->seld = true;
			}
		}
	}
	else {
		UI::Quad(pos.x - 1, pos.y, pos2.x + 2, 16 * (n+2) + 1.f, black(0.7f));

		if (Engine::Button(pos.x, pos.y, pos2.x, 16, white(1, 0.2f), "None", 12, white()) == MOUSE_RELEASE) {
			(*dt->target) = 0;
		}
		for (uint a = 0; a < n; ++a) {
			if (Engine::Button(pos.x, pos.y + 16 * (a+1), pos2.x, 16, white(1, 0.2f), dt->list[a], 12, white()) == MOUSE_RELEASE) {
				(*dt->target) ^= 1<<a;
			}
			if (!!((*dt->target) & 1<<a)) {
				UI::Texture(pos.x + pos2.x - 17, pos.y + 16 * (a + 1), 16, 16, Icons::tick);
			}
		}
		if (Engine::Button(pos.x, pos.y + 16 * (n+1), pos2.x, 16, white(1, 0.2f), "All", 12, white()) == MOUSE_RELEASE) {
			(*dt->target) = (1<<n) - 1;
		}
		if ((*dt->target) == (1 << n) - 1) {
			UI::Texture(pos.x + pos2.x - 17, pos.y + 16 * (n + 1), 16, 16, Icons::tick);
		}
		n += 2;
	}
	if ((Input::mouse0State == 1) && !Engine::Button(pos.x, pos.y, pos2.x, 16.f*(n+2))) {
		Popups::type = POPUP_TYPE::NONE;
	}
}