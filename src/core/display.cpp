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

#include "ChokoLait.h"

int Display::width = 256, Display::height = 256;
int Display::actualWidth = 256, Display::actualHeight = 256;
int Display::frameWidth = 256, Display::frameHeight = 256;

float Display::dpiScl = 1;

NativeWindow* Display::window = nullptr;
CURSORTYPE Display::cursorType;

void Display::OnDpiChange() {
	ChokoLait::ReshapeGL(window, actualWidth, actualHeight);
	Scene::dirty = true;
	UI::font.ClearGlyphs();
	if (UI::font2) UI::font2.ClearGlyphs();
}

void Display::Resize(int x, int y, bool maximize) {
	glfwSetWindowSize(window, x, y);
}

void Display::SetCursor(CURSORTYPE type) {
	cursorType = type;
}