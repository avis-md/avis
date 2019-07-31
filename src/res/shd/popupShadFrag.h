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
	const char popupShadFrag[] = R"(
#version 330

uniform vec2 screenSize;
uniform float distance, intensity;
uniform vec4 pos; //screen pixels

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize; //screen position xy 0~1
	vec2 d = vec2(abs(gl_FragCoord.x  - (pos.r + 0.5*pos.b)) - 0.5*pos.b, abs(gl_FragCoord.y  - (pos.g + 0.5*pos.a)) - 0.5*pos.a);
	d = max(d/distance, 0);
	
	fragCol = vec4(0, 0, 0, intensity*(1-max(d.x, d.y)));
	//fragCol = vec4(0, 0, 0, abs(gl_FragCoord.y  - (pos.g + 0.5*pos.a)) - 0.5*pos.a);
	
}
)";
}