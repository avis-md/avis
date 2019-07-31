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
	const char coreFrag3[] = R"(
#version 330 core
uniform vec4 col;
layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;
void main(){
	outColor = col;
	outId = uvec4(0,0,0,0);
	outNormal = vec4(0,0,0,0);
}
)";
}