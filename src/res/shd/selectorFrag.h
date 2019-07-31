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
	const char selectorFrag[] = R"(
#version 330 core

uniform vec2 screenSize;
uniform int myId;
uniform usampler2D idTex;
uniform vec3 hlCol;

out vec4 outColor;

void main()
{
	int id = int(texture(idTex, gl_FragCoord.xy / screenSize).x);
	if (id == myId) {
		outColor = vec4(hlCol, 0.2);
		return;
	}

	vec2 uv = (gl_FragCoord.xy - vec2(2, 2)) / screenSize;
	float du = 1.0 / screenSize.x;
	float dv = 1.0 / screenSize.y;

	float ih = 0;
	for (int x = 0; x < 5; x++) {
		for (int y = 0; y < 5; y++) {
			if (x != 2 || y != 2) {
				//idf = floor(texture(idTex, uv + vec2(x * du, y * dv)).x);
				//id = int(idf);
				int id2 = int(texture(idTex, uv + vec2(x * du, y * dv)).x);
				if (id2 == myId) {
					ih = 1;
					break;
				}
			}
		}
	}
	outColor = vec4(hlCol, ih);
}
)";
}