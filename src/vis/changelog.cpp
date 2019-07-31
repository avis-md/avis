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

#include "changelog.h"
#include "ui/ui_ext.h"
#include "vis/system.h"

bool ChangeLog::show;
std::vector<std::string> ChangeLog::logs;

void ChangeLog::Init() {
	auto txt = IO::GetText(VisSystem::localFd + "lastversion");
	if (txt != APPVERSION) {
		IO::WriteFile(VisSystem::localFd + "lastversion", APPVERSION);
		show = true;
	}
	else show = false;
	auto s = IO::GetText(IO::path + "CHANGELOG");
	logs = string_split(s, '\n');
}

void ChangeLog::Draw() {
	if (!show) return;
	UI::IncLayer();
	UI::Quad(0, 0, (float)Display::width, (float)Display::height, black(0.4f));
	UI2::BackQuad(Display::width/2 - 200.f, Display::height/2 - 250.f, 400, 500);
	UI::Label(Display::width/2 - 195.f, Display::height/2 - 247.f, 20, "Change Log " APPVERSION, white());
	float off = UI::BeginScroll(Display::width/2 - 195.f, Display::height/2 - 215.f, 390, 455);
	for (auto& l : logs) {
		if (l[0] == '#') {
			UI::Label(Display::width/2 - 193.f, off, 14, l.substr(1), white());
			off += 18;
		}
		else {
			UI::Label(Display::width/2 - 193.f, off, 12, l, white());
			off += 16;
		}
	}
	UI::EndScroll(off);
	if ((UI::_layer == UI::_layerMax) && (Input::KeyDown(KEY::Escape) || (Input::mouse0State == 1 && !Rect(Display::width/2 - 200.f, Display::height/2 - 250.f, 400, 500).Inside(Input::mousePos)))) {
		show = false;
	}
}