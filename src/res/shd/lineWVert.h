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
    const char lineWVert[] = R"(
#version 330

uniform samplerBuffer poss;
uniform vec2 width;
uniform mat4 MVP;

void main() {
    int vid = gl_VertexID / 6;
    int vof = int(mod(gl_VertexID, 6));

    vec3 p1 = texelFetch(poss, vid).rgb;
    vec3 p2 = texelFetch(poss, vid + 1).rgb;

    vec4 sp1 = MVP * vec4(p1, 1);
	sp1 /= sp1.w;
    vec4 sp2 = MVP * vec4(p2, 1);
	sp2 /= sp2.w;

    vec2 dir = normalize(sp2.xy-sp1.xy);
    vec2 nrm = vec2(dir.y * width.x, -dir.x * width.y);

    gl_Position = mix(sp1, sp2, mod(vof, 2));
    if (vof > 1 && vof < 5) gl_Position.xy += nrm;
    else gl_Position.xy -= nrm;
}
)";
}