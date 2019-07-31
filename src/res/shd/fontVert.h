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
	const char fontVert[] = R"(
#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in int c;

uniform vec2 off;
uniform int mask;

out vec2 UV;

void main() {
	int cc = c & 0x00ff;
	int mk = c & 0xff00;
	if (mk == mask) {
		gl_Position.xyz = (pos + vec3(off, 0))*2 - vec3(1,1,0);
		UV = vec2(mod(cc, 16) + mod(gl_VertexID, 2), (cc/16) + 1 - floor(mod(gl_VertexID, 4)/2))/16;
	}
	else {
		gl_Position.xyz = vec3(-2, -2, 0);
		UV = vec2(0, 0);
	}
	gl_Position.w = 1;
}
)";
}