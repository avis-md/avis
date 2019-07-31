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
#include "ui_ext.h"
#include "icons.h"
#include "popups.h"
#include "utils/dialog.h"
#include "res/shd/bezierVert.h"
#include "res/shd/bezierFrag.h"
#include "vis/system.h"

float UI2::sepw = 0.5f;
#define sepw2 (1-sepw)

Shader UI2::bezierProg;

UniqueCallerList UI2::tooltipCallee;
float UI2::tooltipX, UI2::tooltipY;
std::string UI2::tooltipStr;
long long UI2::tooltipTime;

void UI2::Init() {
	(bezierProg = Shader::FromVF(glsl::bezierVert, glsl::bezierFrag))
		.AddUniforms({ "pts", "count", "thick", "col1", "col2" });

#define LC(nm) bezierProg.AddUniform(#nm)
	LC(pts);
	LC(count);
	LC(thick);
	LC(col);
#undef LC
}

void UI2::PreLoop() {
	tooltipCallee.Preloop();
	if (tooltipX == -10) {
		tooltipCallee.Clear();
		tooltipTime = 0;
	}
	else tooltipX = -10;
	tooltipStr.clear();
}

void UI2::DrawTooltip() {
	if (tooltipTime > 0 && !!tooltipStr.size()) {
		const auto a = std::min((Time::millis - tooltipTime - 500) / 200.f, 1.f);
		const auto mw = UI::GetLabelW(12, tooltipStr);
		UI::Quad(tooltipX, tooltipY - 21, mw + 6, 20, black(a * 0.7f));
		UI::Quad(tooltipX + 1, tooltipY - 20, mw + 4, 18, white(a * 0.7f, 0.15f));
		UI::Label(tooltipX + 3, tooltipY - 18, 12, tooltipStr, white(a, 0.9f));
	}
}

void UI2::LabelMul(float x, float y, float sz, const std::string& s) {
	auto ss = string_split(s, '\n');
	for (size_t a = 0; a < ss.size(); ++a) {
		UI::Label(x, std::roundf(y + sz * 1.2f * a), sz, ss[a], white());
	}
}

std::string UI2::EditText(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled, Vec4 col) {
	UI::Label(x, y, 12, title, white());
	if (enabled) {
		return UI::EditText(x + w*sepw, y, w*sepw2 - 1.f, 16, 12, col, val, true, white());
	}
	else {
		Engine::Button(x + w*sepw, y, w*sepw2 - 1.f, 16, col, val, 12, white(0.5f));
		return val;
	}
}

std::string UI2::EditPass(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled, Vec4 col) {
	UI::Label(x, y, 12, title, white());
	if (enabled) {
		return UI::EditTextPass(x + w*sepw, y, w*sepw2 - 1.f, 16, 12, col, val, '*', true, white());
	}
	else {
		Engine::Button(x + w*sepw, y, w*sepw2 - 1.f, 16, col, "****", 12, white(0.5f));
		return val;
	}
}

int UI2::SliderI(float x, float y, float w, const std::string& title, int a, int b, int t, const std::string& desc, const std::string& lbl) {
	if (desc != "") {
		Tooltip(Engine::Button(x, y, w, 16), x, y, desc);
	}
	UI::Label(x, y, 12, title, white());
	return (int)Slider(x + w*sepw, y, w*sepw2 - 1.f, (float)a, (float)b, (float)t, (lbl == "\1")? std::to_string(t) : lbl);
}

float UI2::Slider(float x, float y, float w, const std::string& title, float a, float b, float t, const std::string& desc, const std::string& lbl) {
	if (desc != "") {
		Tooltip(Engine::Button(x, y, w, 16), x, y, desc);
	}
	UI::Label(x, y, 12, title, white());
	return Slider(x + w*sepw, y, w*sepw2 - 1.f, a, b, t, lbl);
}

float UI2::Slider(float x, float y, float w, float a, float b, float t, const std::string& lbl) {
	t = Engine::DrawSliderFill(x, y, w, 16, a, b, t, white(1, 0.5f), white());
	UI::Label(x + 2, y, 12, (lbl == "\1")? std::to_string(t) : lbl, white(1, 0.2f));
	return t;
}

void UI2::Color(float x, float y, float w, Vec4& col) {
	if (Engine::Button(x, y, w, 16, col) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::COLORPICK;
		Popups::pos = Vec2(x + w*sepw, y + 16);
		Popups::pos2 = Vec2(x + w, y);
		Popups::data = &col;
	}
	UI::Texture(x + w - 18, y, 16, 16, Icons::colorwheel);
}

void UI2::Color(float x, float y, float w, const std::string& title, Vec4& col) {
	UI::Label(x, y, 12, title, white());
	Color(x + w*sepw, y, w*sepw2 - 1.f, col);
	UI::Texture(x + w - 18, y, 16, 16, Icons::colorwheel);
}

void UI2::File(float x, float y, float w, const std::string& title, const std::string& fl, filecallback func) {
	UI::Label(x, y, 12, "File", white());
	if (Engine::Button(x + w*sepw, y, w*sepw2 - 1.f, 16, white(1, 0.3f), fl, 12, white(0.5f)) == MOUSE_RELEASE) {
		std::vector<std::string> exts = {"*.hdr"};
		auto res = Dialog::OpenFile(exts);
		if (!!res.size()) {
			func(res);
		}
	}
}

MOUSE_STATUS UI2::Button2(float x, float y, float w, const std::string& s, const Texture& tex, Vec4 col, Vec4 col2) {
	auto ret = Engine::Button(x, y, w, 16, col);
	UI::Texture(x + 1, y, 16, 16, tex, col2);
	UI::font.Align(ALIGN_TOPCENTER);
	UI::Label(x + 9 + w/2, y, 12, s, col2);
	UI::font.Align(ALIGN_TOPLEFT);
	return ret;
}

void UI2::Dropdown(float x, float y, float w, const std::string& title, const Popups::DropdownItem& data) {
	UI::Label(x, y, 12, title, white());
	Dropdown(x + w*sepw, y, w*sepw2 - 1, data);
}

void UI2::Dropdown(float x, float y, float w, const Popups::DropdownItem& data, std::function<void()> func, std::string label, Vec4 col) {
	if (label[0] == 1) {
		if (!data.flags)
			label = data.list[*data.target];
		else {
			if (!*data.target) label = "None";
			else {
				int a = 0; int j = 0;
				while (data.list[a] != "") {
					if ((*data.target & (1 << a))>0) {
						if (!j) {
							label = data.list[a];
							j = 1;
						}
						else if (j < 3) {
							label += ", " + data.list[a];
							j++;
						}
						else {
							label += ", ...";
							break;
						}
					}
					a++;
				}
			}
		}
	}
	if (Engine::Button(x, y, w, 16, white(1, 0.3f), label, 12, col) == MOUSE_RELEASE) {
		if (func) func();
		Popups::type = POPUP_TYPE::DROPDOWN;
		Popups::pos = Vec2(x, y + 16);
		Popups::pos2.x = w;
		Popups::data = (Popups::DropdownItem*)&data;
	}
	UI::Texture(x + w - 16, y, 16, 16, Icons::dropdown2);
}

void UI2::Toggle(float x, float y, float w, const std::string& title, bool& val) {
	UI::Label(x, y, 12, title, white());
	val = Engine::Toggle(x + w - 16, y, 16, Icons::checkbox, val, white(), ORIENT_HORIZONTAL);
}

void UI2::Switch(float x, float y, float w, const std::string& title, int c, std::string* nms, int& i) {
	UI::Label(x, y, 12, title, white());
	x += w*sepw;
	float dw = w*sepw2 / c;
	for (int a = 0; a < c; ++a) {
		if (Engine::Button(x + dw * a, y, dw - 1, 16, white(1, (a == i) ? 0.1f : 0.3f), nms[a], 12, white(), true) == MOUSE_RELEASE) {
			i = a;
		}
	}
}

Vec2 UI2_DS(float x, float y) {
	const float dw = 1.f / Display::width;
	const float dh = 1.f / Display::height;
	auto res = (UI::matrixIsI? Vec2(x, y) : Vec2(UI::matrix * Vec3(x, y, 1.f)));
	return Vec2(res.x * dw, 1 - res.y * dh);
}

Vec2 UI2_DS(Vec2 v) {
	const float dw = 1.f / Display::width;
	const float dh = 1.f / Display::height;
	auto res = (UI::matrixIsI? v : Vec2(UI::matrix * Vec3(v, 1.f)));
	return Vec2(res.x * dw, 1 - res.y * dh);
}

void UI2::Bezier(Vec2 p1, Vec2 t1, Vec2 t2, Vec2 p2, Vec4 col, float thick, int reso) {
	Bezier(p1, t1, t2, p2, col, col, thick, reso);
}

void UI2::Bezier(Vec2 p1, Vec2 t1, Vec2 t2, Vec2 p2, Vec4 col1, Vec4 col2, float thick, int reso) {
	thick /= 2;
	bezierProg.Bind();
	Vec2 pts[4] = { UI2_DS(p1) * 2.f - 1.f, UI2_DS(t1) * 2.f - 1.f, 
					UI2_DS(t2) * 2.f - 1.f, UI2_DS(p2) * 2.f - 1.f };
	Vec2 tt = (UI::matrixIsI? Vec2(thick, thick) : Vec2(UI::matrix * Vec3(thick, thick, 0.f)));
	glUniform2fv(bezierProg.Loc(0), 4, &pts[0][0]);
	glUniform1i(bezierProg.Loc(1), reso);
	glUniform2f(bezierProg.Loc(2), tt.x / Display::width, tt.y / Display::height);
	glUniform4f(bezierProg.Loc(3), col1.r, col1.g, col1.b, col1.a);
	glUniform4f(bezierProg.Loc(4), col2.r, col2.g, col2.b, col2.a);
	glBindVertexArray(Camera::emptyVao);
	glDrawArrays(GL_TRIANGLES, 0, reso * 6);
	glUseProgram(0);
}

float UI2::Scroll(float x, float y, float h, float t, float tot, float fill) {
	static float scrpos = -1;

	bool me = Input::mouse0 && (UI::_layer == UI::_layerMax) && Rect(x, y, 8, h).Inside(Input::mouseDownPos);
	UI::Quad(x, y, 8, h, black(0.2f));
	auto scr = Engine::Button(x, y + (h - 2)*t / tot, 8, (h-2)*fill/tot + 2);
	if (scr > 0 || (me && scrpos >= 0)) {
		UI::Quad(x, y + (h - 2)*t / tot, 8, (h - 2)*fill / tot + 2, (scr == MOUSE_HOVER_FLAG) ? white() : white(1, 0.5f));
		if (scr == MOUSE_CLICK) scrpos = Input::mousePos.y - (y + (h - 2)*t / tot);
		else if (scr == MOUSE_RELEASE) scrpos = -1;
		else if (me && scrpos >= 0) {
			t = std::min((Input::mousePos.y - scrpos - y) / (h - 2) * tot, tot - fill);
			t = std::max(t, 0.f);
		}
	}
	else {
		UI::Quad(x + 1, y + 1 + (h - 2)*t / tot, 6, (h - 2)*fill / tot, white(0.5f));
	}
	return t;
}

void UI2::BlurQuad(float x, float y, float w, float h, Vec4 tint) {
	UI::Quad(x, y, w, h, ChokoLait::mainCamera->blitTexs[1], tint, 0,
		UI2_DS(x, y), 
		UI2_DS(x + w, y), 
		UI2_DS(x, y + h), 
		UI2_DS(x + w, y + h));
}

void UI2::BackQuad(float x, float y, float w, float h, Vec4 col) {
	if (col.r < 0) col = VisSystem::backColor;
	else col *= VisSystem::backColor;
	if (VisSystem::blur && VisSystem::opacity < 1) {
		BlurQuad(x, y, w, h);
	}
	UI::Quad(x, y, w, h, white(VisSystem::opacity) * col);
}

void UI2::BackQuadC(float x, float y, float w, float h, Vec4 col) {
	if (VisSystem::blur && col.a < 1) {
		BlurQuad(x, y, w, h);
	}
	UI::Quad(x, y, w, h, col);
}

Vec3 UI2::EditVec(float x, float y, float w, const std::string& t, Vec3 v, bool ena) {
	Vec3 res;
	res.x = TryParse(UI2::EditText(x, y, w, t + " X", std::to_string(v.x), ena, Vec4(0.6f, 0.4f, 0.4f, 1)), 0.f);
	res.y = TryParse(UI2::EditText(x, y + 17, w, t + " Y", std::to_string(v.y), ena, Vec4(0.4f, 0.6f, 0.4f, 1)), 0.f);
	res.z = TryParse(UI2::EditText(x, y + 34, w, t + " Z", std::to_string(v.z), ena, Vec4(0.4f, 0.4f, 0.6f, 1)), 0.f);
	return res;
}

MOUSE_STATUS UI2::Tooltip(MOUSE_STATUS status, float x, float y, const std::string str) {
	if (!!(status & MOUSE_HOVER_FLAG)) {
		tooltipX = x;
		tooltipY = y;
		if (tooltipCallee.Add() && !Input::mouse0) {
			if (Time::millis - tooltipTime > 500) {
				tooltipStr = str;
			}
		}
		else {
			tooltipCallee.Set();
			tooltipTime = Time::millis;
		}
	}
	return status;
}