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
	const char fogFrag[] = R"(
#version 330

uniform vec2 screenSize;
uniform sampler2D inColor;
uniform sampler2D inDepth;
float fogStr;
float fogSkyStr;
vec3 skyDir;
vec4 fogCol;
vec4 fogSkyCol;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize; //screen position xy 0~1
	vec4 col = texture(inColor, uv);
	float lumi = 0.23*col.r + 0.66*col.g + 0.11*col.b;
	float r = clamp((lumi*2 - 1)*3, 0, 1);
	float g = 0;
	float b = 0;	
	if (lumi*6 > 5) {
		b = max(lumi*6-5, 0);
		g = b;
	}
	else {
		b = min((1-abs(1-2*min(lumi*2, 1)))*1.33, 1);
		g = min((1-abs(1-2*min((lumi - 1.0/6.0)*1.5, 1)))*2, 1);
	}
	fragCol = vec4(r, g, b, 1);
}
)";
}