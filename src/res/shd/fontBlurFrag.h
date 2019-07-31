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
	const char fontBlurFrag[] = R"(
#version 330

uniform sampler2D tex;
uniform int size;
uniform int rad;
uniform bool isY;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / size;
	vec2 ct = (floor(uv * 16.0) + 0.5) / 16;
	vec2 dp = (uv - ct);
	float kernal[7] = float[]( 0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598);
	fragCol = vec4(0,0,0,1);
	for (int a = 0; a < 7; ++a) {
		float xx = (a-3.0) / size;
		float dd;
		if (isY) {
			dd = dp.y + xx;
		}
		else {
			dd = dp.x + xx;
		}
		float col = 0;
		if ((dd > (-0.5 / 16)) && (dd < (0.5 / 16))) {
			if (isY)
				col = texture(tex, vec2(uv.x, ct.y + dd)).r;
			else
				col = texture(tex, vec2(ct.x + dd, uv.y)).r;
		}
		
		fragCol.r += (col * kernal[a]);
	}
}
)";
}