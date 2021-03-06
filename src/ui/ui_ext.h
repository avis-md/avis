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
#include "popups.h"
#include "utils/uniquecaller.h"
#include <functional>

//UI with default style settings

class UI2 {
public:
	typedef void(*filecallback) (std::vector<std::string>);

	static void Init();
	static void PreLoop();
	static void DrawTooltip();

	static float sepw;

	static void LabelMul(float x, float y, float sz, const std::string& s);
	static std::string EditText(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled = true, Vec4 col = white(1, 0.5f));
	static std::string EditPass(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled = true, Vec4 col = white(1, 0.5f));
	static int SliderI(float x, float y, float w, const std::string& title, int a, int b, int t, const std::string& desc = "", const std::string& lbl = "\1");
	static float Slider(float x, float y, float w, const std::string& title, float a, float b, float t, const std::string& desc = "", const std::string& lbl = "\1");
	static float Slider(float x, float y, float w, float a, float b, float t, const std::string& lbl = "\1");
	static void Color(float x, float y, float w, Vec4& col);
	static void Color(float x, float y, float w, const std::string& title, Vec4& col);
	static void File(float x, float y, float w, const std::string& title, const std::string& fl, filecallback func);
	static MOUSE_STATUS Button2(float x, float y, float w, const std::string& s, const Texture& tex, Vec4 col = white(1, 0.4f), Vec4 col2 = white());
	static void Dropdown(float x, float y, float w, const std::string& title, const Popups::DropdownItem& data);
	static void Dropdown(float x, float y, float w, const Popups::DropdownItem& data, std::function<void()> func = nullptr, std::string label = "\1", Vec4 col = white());
	static void Toggle(float x, float y, float w, const std::string& title, bool& val);
	static void Switch(float x, float y, float w, const std::string& title, int c, std::string* nms, int& i);
	static void Bezier(Vec2 p1, Vec2 t1, Vec2 t2, Vec2 p2, Vec4 col, float thick = 1, int reso = 20);
	static void Bezier(Vec2 p1, Vec2 t1, Vec2 t2, Vec2 p2, Vec4 col1, Vec4 col2, float thick = 1, int reso = 20);
	static float Scroll(float x, float y, float h, float t, float tot, float fill);

	static void BlurQuad(float x, float y, float w, float h, Vec4 tint = white());
	static void BackQuad(float x, float y, float w, float h, Vec4 col = Vec4(-1));
	static void BackQuadC(float x, float y, float w, float h, Vec4 col);

	static Vec3 EditVec(float x, float y, float w, const std::string& title, Vec3 v, bool ena);

	static MOUSE_STATUS Tooltip(MOUSE_STATUS status, float x, float y, const std::string str);

	static Shader bezierProg;

private:
	static UniqueCallerList tooltipCallee;
	static float tooltipX, tooltipY;
	static std::string tooltipStr;
	static long long tooltipTime;
};