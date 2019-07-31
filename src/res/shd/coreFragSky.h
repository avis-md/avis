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
    const char coreFragSky[] = R"(
#version 330 core
in vec2 UV;
uniform sampler2D sampler;
uniform vec2 dir;
uniform float length;
out vec4 color;
void main(){
	float ay = asin((UV.y) / length);
	float l2 = length*cos(ay);
	float ax = asin((dir.x + UV.x) / l2);
	color = textureLod(sampler, vec2((dir.x + ax / 3.14159)*sin(dir.y + ay / 3.14159) + 0.5, (dir.y + ay / 3.14159)), 0);
	color.a = 1;
}
)";
}