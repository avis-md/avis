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

#include "help.h"
#include "vis/pargraphics.h"
#include "utils/tinyfiledialogs.h"
#include "ui/icons.h"

const std::string _opn[] = {
	"Open file via terminal",
	"	Command line is: avis filename [args...]",
	"	Args (default values are in brackets)",
	"		-d[0/(1)]		Show the import dialog.",
	"		-t[0/(1)]		Load trajectory if available.",
	"		-T[filename]	Load trajectory file. If this value is specified, -t will be ignored.",
	"		-f[int(0):int(1)]	Maximum number of frames:Number to skip between frames. If number is 0,",
	"						all frames are read. Eg: -f3:2 to load frames 0, 2, and 4.",
	"		-b[0/(1)]		Calculate bonds",
	"		-c[0/(1)/2]		If bond cache is available, 0=ignore, 1=use, 2=recalculate and override.",
	"						If no cache is available and value is NOT 0, cache will be generated.",
	"						Default value will be 2 if molecular file is older than cache."
};

bool HelpMenu::show = false;
float HelpMenu::alpha = 0;

void HelpMenu::Draw() {
	if (!UI::editingText) {
		if (!show && Input::KeyDown(KEY::F1)) show = true;
		if (show && Input::KeyDown(KEY::Escape)) show = false;
	}
	if (alpha > 0) {
		UI::IncLayer();
		UI::alpha = alpha;

		UI::Quad(0, 0, static_cast<float>(Display::width), static_cast<float>(Display::height), black(0.9f));
		UI::Label(10, 5, 14, "HELP (?)", white());
		UI::font.Align(ALIGN_TOPRIGHT);
		UI::Label(Display::width - 5.f, 5, 10, "version 0.01", white(0.7f));
		UI::font.Align(ALIGN_TOPLEFT);

		//for (uint a = 0; a < 12; ++a) {
		//	UI::Label(20, 25 + 15 * a, 12, _opn[a], white());
		//}

		UI::Label(10, Display::height - 16.f, 12, "This program is under development.", white(0.7f));
		UI::alpha = 1;
	}
	alpha = Clamp(alpha + (show? 1 : -1)*5*Time::delta, 0.f, 1.f);
}

void HelpMenu::Link(float x, float y, const std::string& path) {
	if (Engine::Button(x + 1, y + 1, 14, 14, Icons::help) == MOUSE_RELEASE) {
		IO::OpenEx(IO::path + "docs/reference/" + path);
	}
}