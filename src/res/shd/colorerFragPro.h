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
	const char colererFragPro[] = R"(
#version 330

uniform usampler2D idTex;
uniform vec2 screenSize;
uniform int proId;
uniform vec4 col;

layout (location = 0) out vec4 fragCol;

void main () {
	uvec4 tx = texture(idTex, gl_FragCoord.xy / screenSize);
	if (int(tx.y) == (proId + 65536)) {
		fragCol = col;
		return;
	}
	else discard;
}
)";
}