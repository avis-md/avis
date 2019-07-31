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
    const char bezierVert[] = R"(
#version 330 core
uniform vec2 pts[4];
uniform int count;
uniform vec2 thick;

out float t;

vec2 ptat(float t) {
	float i = 1 - t;
	return i*i*i*pts[0] + 3*i*i*t*pts[1] + 3*i*t*t*pts[2] + t*t*t*pts[3];
}

void main(){
	int i = int(mod(gl_VertexID, 6));
	int ti = int(floor(gl_VertexID * 0.16667f)) + int(mod(i, 2));
	float dt = 1.0 / count;
	t = ti * dt;
	
	vec2 here = ptat(t);
	vec2 dir = ptat(t + dt) - ptat(t - dt);
	vec2 nrm = normalize(vec2(-dir.y, dir.x));

	if (i < 2 || i == 5) {
		gl_Position.xy = here + thick * nrm;
	}
	else {
		gl_Position.xy = here - thick * nrm;
	}
	gl_Position.z = gl_Position.w = 1.0;
}
)";
}