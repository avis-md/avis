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

//shorthands
Vec4 black(float f = 1);
Vec4 red(float f = 1, float i = 1), green(float f = 1, float i = 1), blue(float f = 1, float i = 1), cyan(float f = 1, float i = 1), yellow(float f = 1, float i = 1), white(float f = 1, float i = 1);

class Color {
public:
	static void Init();

	static GLuint pickerProgH, pickerProgH2, pickerProgSV;
	static GLint pickerProgH2Locs[5], pickerProgSVLocs[5];

	static void Rgb2Hsv(byte r, byte g, byte b, float& h, float& s, float& v), Hsv2Rgb(float h, float s, float v, byte& r, byte& g, byte& b);
	static Vec3 Rgb2Hsv(Vec4 col);
	static std::string Col2Hex(Vec4 col), Col2Hex(byte* bs);
	static void DrawPicker(bool hasA = true);
	static Vec4 HueBaseCol(float hue);

	static void DrawSV(float x, float y, float w, float h, float hue);
	static void DrawH(float x, float y, float w, float h);
	static void DrawH2(float x, float y, float w, float h, Vec4* grad);
};