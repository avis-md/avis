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
    const char blurFrag[] = R"(
#version 330

in vec2 uv;

uniform sampler2D mainTex;
uniform float mul;
uniform vec2 screenSize;
uniform float isY;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize;
	float kernal[21] = float[]( 0.011, 0.0164, 0.023, 0.031, 0.04, 0.05, 0.06, 0.07, 0.076, 0.08, 0.0852, 0.08, 0.076, 0.07, 0.06, 0.05, 0.04, 0.031, 0.023, 0.0164, 0.011 );
	
	fragCol = vec4(0,0,0,0);
	for (int a = 0; a < 21; ++a) {
		int xx = a-10;
		vec2 uv2;
		if (isY == 0) uv2 = uv + vec2(xx*mul/screenSize.x, 0);
		else uv2 = uv + vec2(0, xx*mul/screenSize.y);
		fragCol += (texture(mainTex, uv2) * kernal[a]);
	}
}
)";
}