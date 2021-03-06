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

#include "Engine.h"

bool Input::mouse0 = false, Input::mouse1 = false, Input::mouse2 = false;
bool Input::_mouse0 = false, Input::_mouse1 = false, Input::_mouse2 = false;
long long Input::mouseT = false;
bool Input::dbclick = false;
float Input::mouseScroll = 0;
byte Input::mouse0State = 0, Input::mouse1State = 0, Input::mouse2State = 0;
byte Input::_mouse0State = 0;
std::string Input::inputString = "";

float Input::scrollScl = 1;

Vec2 Input::mousePos = Vec2(0, 0);
Vec2 Input::mousePosRelative = Vec2(0, 0);
Vec2 Input::mousePosOld = Vec2(0, 0);
Vec2 Input::mouseDelta = Vec2(0, 0);
Vec2 Input::mouseDownPos = Vec2(0, 0);

bool Input::keyStatusOld[] = {};
bool Input::keyStatusNew[] = {};

std::string Input::_inputString = "";
float Input::_mouseScroll = 0;

void Input::RegisterCallbacks() {
	glfwSetInputMode(Display::window, GLFW_STICKY_KEYS, 1);
	glfwSetCharCallback(Display::window, TextCallback);
}

void Input::TextCallback(GLFWwindow* w, uint i) {
	_inputString += std::string((char*)&i, 1);
}

bool Input::KeyDown(KEY k) {
	return keyStatusNew[(int)k - 32] && !keyStatusOld[(int)k - 32];
}

bool Input::KeyHold(KEY k) {
	return keyStatusNew[(int)k - 32];
}

bool Input::KeyUp(KEY k) {
	return !keyStatusNew[(int)k - 32] && keyStatusOld[(int)k - 32];
}

#define SV(val) _ ## val = val
#define LD(val) val = _ ## val

void Input::UpdateMouseNKeyboard(bool* src) {
	memcpy(keyStatusOld, keyStatusNew, 325);
	if (src) std::swap_ranges(src, src + 325, keyStatusNew);
	else {
		for (uint i = 32; i < 97; ++i) {
			keyStatusNew[i - 32] = !!glfwGetKey(Display::window, i);
		}
		for (uint i = 256; i < 347; ++i) {
			keyStatusNew[i - 32] = !!glfwGetKey(Display::window, i);
		}
	}
	
	LD(mouse0);
	LD(mouse1);
	LD(mouse2);

	LD(mouse0State);
	if (mouse0)
		mouse0State = std::min<byte>(mouse0State + 1U, MOUSE_HOLD);
	else
		mouse0State = ((mouse0State == MOUSE_UP) | (mouse0State == 0)) ? 0 : MOUSE_UP;
	if (mouse1)
		mouse1State = std::min<byte>(mouse1State + 1U, MOUSE_HOLD);
	else
		mouse1State = ((mouse1State == MOUSE_UP) | (mouse1State == 0)) ? 0 : MOUSE_UP;
	if (mouse2)
		mouse2State = std::min<byte>(mouse2State + 1U, MOUSE_HOLD);
	else
		mouse2State = ((mouse2State == MOUSE_UP) | (mouse2State == 0)) ? 0 : MOUSE_UP;
		
	SV(mouse0State);

	if (!mouse0State)
		mouseDownPos = Vec2(-1, -1);
	else if (mouse0State == MOUSE_DOWN) {
		mouseDownPos = mousePos;
		auto mt = Time::millis;
		dbclick = (mt - mouseT < 300) && !dbclick;
		mouseT = mt;
	}

	mouseDelta = mousePos - mousePosOld;
	mousePosOld = mousePos;

	if (KeyHold(KEY::LeftControl) && KeyDown(KEY::V)) {
		_inputString += glfwGetClipboardString(Display::window);
	}
}

void Input::PreLoop() {
	inputString = _inputString;
	_inputString.clear();
	mouseScroll = _mouseScroll;
	_mouseScroll = 0;
}

bool Input::CheckCopy(const char* loc, size_t len) {
	if (KeyHold(KEY::LeftControl)) {
		if (KeyDown(KEY::X)) {
			glfwSetClipboardString(Display::window, std::string(loc, len).c_str());
			return true;
		}
		else if (KeyDown(KEY::C)) {
			glfwSetClipboardString(Display::window, std::string(loc, len).c_str());
			return false;
		}
	}
	return false;
}