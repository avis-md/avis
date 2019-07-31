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

enum CURSORTYPE : byte {
	CURSORTYPE_NORMAL,
	CURSORTYPE_TEXT,
	CURSORTYPE_SELECT
};

/*! Display window properties
[av] */
class Display {
public:
	static int width, height;
	static int actualWidth, actualHeight;
	static int frameWidth, frameHeight;
	static glm::mat3 uiMatrix;
	static bool uiMatrixIsI;

	static float dpiScl;
	
	static void OnDpiChange();
	static void Resize(int x, int y, bool maximize = false);

	static void SetCursor(CURSORTYPE);

	friend int main(int argc, char **argv);
	//move all functions in here please
	friend void MouseGL(GLFWwindow* window, int button, int state, int mods);
	friend void MotionGL(GLFWwindow* window, double x, double y);
	friend void FocusGL(GLFWwindow* window, int focus);
	friend class PopupSelector;
	friend class ChokoLait;

	static NativeWindow* window;
	static CURSORTYPE cursorType;
};