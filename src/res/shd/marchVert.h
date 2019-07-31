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
	const char marchVert[] = R"(#version 330 core
uniform ivec3 shp;

flat out ivec3 id;

void main() {
	int yz = (shp.y-1) * (shp.z-1);
	id.x = gl_VertexID / yz;
	int ty = gl_VertexID - id.x * yz;
	id.y = ty / (shp.z-1);
	id.z = int(mod(ty, shp.z-1));
})";}