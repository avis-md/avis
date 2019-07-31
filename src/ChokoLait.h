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

/*
forked from ChokoLait -- Chokomancarr 2018
*/

#include "Engine.h"

typedef void(*emptyCallbackFunc)(void);
typedef bool(*dropFileFunc)(int, const char**);

class ChokoLait {
public:
	ChokoLait() {
		if (!initd) {
			_InitVars();
			initd = 1;
		}
	}

	static pSceneObject mainCameraObj;
	static pCamera mainCamera;
	static GLFWwindow* window;
	static bool foreground;

	static std::vector<dropFileFunc> dropFuncs;
	static std::vector<emptyCallbackFunc> focusFuncs;

	static void Init(int scrW, int scrH);

	static bool alive();

	static void Update(emptyCallbackFunc func = 0);
	static void Paint(emptyCallbackFunc rendFunc = 0, emptyCallbackFunc paintFunc = 0);

	friend class Display;
protected:
	static int initd;

	static void _InitVars();

	static void MouseGL(GLFWwindow* window, int button, int state, int mods);
	static void MouseScrGL(GLFWwindow* window, double xoff, double yoff);
	static void MouseEnterGL(GLFWwindow* window, int e);
	static void MotionGL(GLFWwindow* window, double x, double y);
	static void ReshapeGL(GLFWwindow* window, int w, int h);
	static void DropGL(GLFWwindow* window, int n, const char** fs);
	static void FocusGL(GLFWwindow* window, int focus);
};