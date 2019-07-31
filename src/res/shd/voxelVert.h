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
	const char voxelVert[] = R"(
#version 330

layout(location=0) in vec3 pos;

uniform mat4 _MV;
uniform mat4 _P;
uniform float size;

out vec3 v2f_uvw;
out vec3 v2f_wpos;

void main(){
	vec4 wp = _MV*vec4(pos * size, 1);
	gl_Position = _P*wp;
	v2f_wpos = (wp / wp.w).xyz;
	v2f_uvw = pos * 0.5 + vec3(0.5, 0.5, 0.5);
	//v2f_uvw = pos * 0.5 / size + vec3(1.0, 1.0, 1.0) / 2;
}
)"; }