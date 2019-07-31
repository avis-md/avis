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
namespace glsl {
	const char colorPickerH[] = R"(
#version 330 core
in vec2 UV;
out vec4 color;
void main(){
	float hue = 6 - UV.y*6;
	vec4 v;

	v.r = clamp(abs(hue - 3) - 1, 0, 1);

	v.g = 1 - clamp(abs(hue - 2) - 1, 0, 1);

	v.b = 1 - clamp(abs(hue - 4) - 1, 0, 1);
	v.a = 1;
	color = v;
}
)"; }