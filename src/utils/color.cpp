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
#include "ui/popups.h"
#include "ui/icons.h"
#include "res/shd/colorPickerVert.h"
#include "res/shd/colorPickerH.h"
#include "res/shd/colorPickerH2.h"
#include "res/shd/colorPickerSV.h"

Vec4 black(float f) { return Vec4(0, 0, 0, f); }
Vec4 red(float f, float i) { return Vec4(i, 0, 0, f); }
Vec4 green(float f, float i) { return Vec4(0, i, 0, f); }
Vec4 blue(float f, float i) { return Vec4(0, 0, i, f); }
Vec4 cyan(float f, float i) { return Vec4(i*0.09f, i*0.706f, i, f); }
Vec4 yellow(float f, float i) { return Vec4(i, i, 0, f); }
Vec4 white(float f, float i) { return Vec4(i, i, i, f); }

GLuint Color::pickerProgH, Color::pickerProgH2, Color::pickerProgSV;
GLint Color::pickerProgH2Locs[], Color::pickerProgSVLocs[];

namespace {

	void Hex2Col(const std::string& s, byte* bs) {
		const char* hx = "0123456789ABCDEF";
		std::string res = "XXXXXXXX";
		
		const auto n = s.size();
		if (!n || n == 2) return;

		std::vector<byte> bb(std::max<size_t>(n, 4), 255);
		for (size_t a = 0; a < n; a++) {
			const auto c = s[a];
			if (c >= '0' && c <= '9') {
				bb[a] = (byte)c - (byte)'0';
			}
			else if (c >= 'A' && c <= 'F') {
				bb[a] = (byte)c - (byte)'A' + 10;
			}
			else if (c >= 'a' && c <= 'f') {
				bb[a] = (byte)c - (byte)'a' + 10;
			}
			else return;
		}
		if (n >= 6) {
			for (int a = 0; a < n/2; a++) {
				bb[a] = (bb[a*2] << 4) + bb[a*2+1];
			}
		}
		
		if (n <= 4) {
			for (int a = 0; a < n; a++) {
				bb[a] = (bb[a] << 4) + bb[a];
			}
			if (n == 1) {
				bb[1] = bb[2] = bb[0];
			}
		}
		for (int a = 0; a < 4; a++) {
			bs[a] = bb[a];
		}
	}
}

void Color::Init() {
	GLuint vs;
	Shader::LoadShader(GL_VERTEX_SHADER, glsl::colorPickerVert, vs);
	pickerProgH = Shader::FromF(vs, glsl::colorPickerH);
	pickerProgH2 = Shader::FromF(vs, glsl::colorPickerH2);
	pickerProgH2Locs[0] = glGetUniformLocation(pickerProgH2, "gradcols");
	pickerProgSV = Shader::FromF(vs, glsl::colorPickerSV);
	pickerProgSVLocs[0] = glGetUniformLocation(pickerProgSV, "col");
	glDeleteShader(vs);
}

void Color::Hsv2Rgb(float h, float s, float v, byte& r, byte& g, byte& b) {
	Vec4 cb = HueBaseCol(h);
	Vec4 c(Lerp(Lerp(cb, Vec4(1, 1, 1, 1), 1 - s), Vec4(), 1 - v));
	r = (byte)std::roundf(c.r * 255);
	g = (byte)std::roundf(c.g * 255);
	b = (byte)std::roundf(c.b * 255);
}

void Color::Rgb2Hsv(byte r, byte g, byte b, float& h, float& s, float& v) {
	float R = r * 0.0039216f;
	float G = g * 0.0039216f;
	float B = b * 0.0039216f;

	float mn = std::min(std::min(R, G), B);
	float mx = std::max(std::max(R, G), B);

	v = mx;
	if (mx > 0) {
		s = (mx - mn) / mx;
		if (mn != mx) {
			if (R == mx) {
				h = (G - B) / (mx - mn);
			}
			else if (G == mx) {
				h = 2.f + (B - R) / (mx - mn);
			}
			else {
				h = 4.f + (R - G) / (mx - mn);
			}
			h /= 6;
			if (h < 0) h += 1;
		}
	}
}

Vec3 Color::Rgb2Hsv(Vec4 col) {
	Vec3 o = Vec3();
	Rgb2Hsv((byte)(col.r * 255), (byte)(col.g * 255), (byte)(col.b * 255), o.r, o.g, o.b);
	return o;
}

std::string Color::Col2Hex(Vec4 col) {
	byte bs[] = { (byte)(col.r * 255), (byte)(col.g * 255), (byte)(col.b * 255), (byte)(col.a * 255) };
	return Col2Hex(bs);
}

std::string Color::Col2Hex(byte* bs) {
	const char* hx = "0123456789ABCDEF";
	std::string res = "XXXXXXXX";
	for (byte a = 0; a < 4; ++a) {
		auto i = bs[a];
		res[a * 2] = hx[i >> 4];
		res[a * 2 + 1] = hx[i & 15];
	}
	return res;
}

void Color::DrawPicker(bool hasA) {
	auto& cl = *((Vec4*)Popups::data);

	Vec3 hsv = Rgb2Hsv(cl);

	if (Popups::pos.x > Display::width - 150) {
		Popups::pos.x = Popups::pos2.x - 150;
	}
	if (Popups::pos.y > Display::height - 150) {
		Popups::pos.y = Popups::pos2.y - 150;
	}

	UI::Quad(Popups::pos.x, Popups::pos.y, 150, 150, black(0.7f));
	Color::DrawSV(Popups::pos.x + 5, Popups::pos.y + 5, 120, 120, hsv.r);
	Vec2 sv(1-hsv.y, hsv.z);
	sv = Engine::DrawSliderFill2D(Popups::pos.x + 5, Popups::pos.y + 5, 120, 120, Vec2(), Vec2(1, 1), sv, white(0), white(0));
	hsv.y = 1-sv.x;
	hsv.z = sv.y;
	UI::Texture(Popups::pos.x + 120 * sv.x, Popups::pos.y + 120 - 120 * sv.y, 10, 10, Icons::circle, (sv.y > 0.5f) ? black() : white());
	UI::Texture(Popups::pos.x + 1 + 120 * sv.x, Popups::pos.y + 121 - 120 * sv.y, 8, 8, Icons::circle, cl);
	Color::DrawH(Popups::pos.x + 132, Popups::pos.y + 5, 15, 120);
	hsv.x = Engine::DrawSliderFillY(Popups::pos.x + 132, Popups::pos.y + 5, 15, 120, 0, 1, hsv.x, white(0), white(0));
	UI::Quad(Popups::pos.x + 130, Popups::pos.y + 4 + 120 * hsv.x, 19, 2, white());

	if ((Input::mouse0State == 1) && !Engine::Button(Popups::pos.x, Popups::pos.y, 150, 150)) {
		Popups::type = POPUP_TYPE::NONE;
	}

	byte bs[4];
	Hsv2Rgb(hsv.x, hsv.y, hsv.z, bs[0], bs[1], bs[2]);
	bs[3] = 255;

	const auto hx = Col2Hex(bs);

	const auto hx2 = UI::EditText(Popups::pos.x + 5, Popups::pos.y + 130, 120, 16, 12, white(1, 0.5f), hx, true, white());

	if (hx2 != hx) {
		Hex2Col(hx2, bs);
	}

	cl = Vec4(bs[0] / 255.f, bs[1] / 255.f, bs[2] / 255.f, 1);
}

Vec4 Color::HueBaseCol(float hue) {
	hue *= 6;
	Vec4 v;
	v.r = Clamp(abs(hue - 3) - 1.f, 0.f, 1.f);
	v.g = 1 - Clamp(abs(hue - 2) - 1.f, 0.f, 1.f);
	v.b = 1 - Clamp(abs(hue - 4) - 1.f, 0.f, 1.f);
	v.a = 1;
	return v;
}

void Color::DrawSV(float x, float y, float w, float h, float hue) {
	Vec3 quadPoss[4];
	quadPoss[0].x = x;
	quadPoss[0].y = y;
	quadPoss[1].x = x + w;
	quadPoss[1].y = y;
	quadPoss[2].x = x;
	quadPoss[2].y = y + h;
	quadPoss[3].x = x + w;
	quadPoss[3].y = y + h;
	for (int y = 0; y < 4; ++y) {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(UI::matrix*quadPoss[y]);
	}

	UI::SetVao(4, quadPoss);

	glUseProgram(pickerProgSV);
	Vec4 bc = HueBaseCol(hue);
	glUniform3f(pickerProgSVLocs[0], bc.r, bc.g, bc.b);

	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Engine::quadBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void Color::DrawH(float x, float y, float w, float h) {
	Vec3 quadPoss[4];
	quadPoss[0].x = x;
	quadPoss[0].y = y;
	quadPoss[1].x = x + w;
	quadPoss[1].y = y;
	quadPoss[2].x = x;
	quadPoss[2].y = y + h;
	quadPoss[3].x = x + w;
	quadPoss[3].y = y + h;
	for (int y = 0; y < 4; ++y) {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(UI::matrix*quadPoss[y]);
	}

	UI::SetVao(4, quadPoss);
	glUseProgram(pickerProgH);
	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Engine::quadBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void Color::DrawH2(float x, float y, float w, float h, Vec4* grad) {
	Vec3 quadPoss[4];
	quadPoss[0].x = x;
	quadPoss[0].y = y;
	quadPoss[1].x = x + w;
	quadPoss[1].y = y;
	quadPoss[2].x = x;
	quadPoss[2].y = y + h;
	quadPoss[3].x = x + w;
	quadPoss[3].y = y + h;
	for (int y = 0; y < 4; ++y) {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(UI::matrix*quadPoss[y]);
	}

	UI::SetVao(4, quadPoss);
	glUseProgram(pickerProgH2);
	glUniform4fv(pickerProgH2Locs[0], 3, &grad[0][0]);
	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Engine::quadBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}