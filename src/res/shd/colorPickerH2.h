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
	const char colorPickerH2[] = R"(
#version 330 core
in vec2 UV;
uniform vec4 gradcols[3];
out vec4 color;
void main(){
	float f = UV.y;
	color.rgb = (gradcols[0].rgb * clamp(2 - 4 * f, 0, 1)
		+ gradcols[1].rgb * clamp(2 - abs(2 - 4 * f), 0, 1)
		+ gradcols[2].rgb * clamp(4 * f - 2, 0, 1));
	color.a = 1;
}
)"; }