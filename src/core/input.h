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

/*! Mouse, keyboard, and touch input
[av] */
class Input {
public:
	static Vec2 mousePos, mousePosRelative, mouseDelta, mouseDownPos;
	static bool mouse0, mouse1, mouse2;
	static bool _mouse0, _mouse1, _mouse2;
	static long long mouseT;
	static bool dbclick;
	static float mouseScroll;
	static byte mouse0State, mouse1State, mouse2State;
	static byte _mouse0State;
	static std::string inputString;
	static void UpdateMouseNKeyboard(bool* src = nullptr);

	static float scrollScl;

	static bool KeyDown(KEY key), KeyHold(KEY key), KeyUp(KEY key);

	static void PreLoop();
	static bool CheckCopy(const char* loc, size_t len);

	friend class ChokoLait;
	friend class Engine;
protected:
	static void RegisterCallbacks(), TextCallback(GLFWwindow*, uint);
	static bool keyStatusOld[325], keyStatusNew[325];
private:
	static Vec2 mousePosOld;
	static std::string _inputString;
	static float _mouseScroll;
};