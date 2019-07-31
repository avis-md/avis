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
    const char dofFrag[] = R"(
#version 330 core

in vec2 uv;

uniform sampler2D mainTex;
uniform sampler2D depthTex;
uniform float plane;
uniform float focal;
uniform float aperture;
uniform vec2 screenSize;
uniform bool isOrtho;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize;
	float kernel[25] = float[25](
		0.003765, 	0.015019, 	0.023792, 	0.015019, 	0.003765,
		0.015019, 	0.059912, 	0.094907, 	0.059912, 	0.015019,
		0.023792, 	0.094907, 	0.150342, 	0.094907, 	0.023792,
		0.015019, 	0.059912, 	0.094907, 	0.059912, 	0.015019,
		0.003765, 	0.015019, 	0.023792, 	0.015019, 	0.003765);

	float z = texture(depthTex, uv).r;
	const float nClip = 0.01;
	const float fClip = 500;
	float zLinear;
    if (isOrtho) zLinear = z;
    else zLinear = (2 * nClip) / (fClip + nClip - z * (fClip - nClip));

	float coc = abs(aperture * (focal * (zLinear - plane)) / (zLinear * (plane - focal)));

	fragCol = vec4(0,0,0,0);
	for (int a = 0; a < 5; ++a) {
		int xx = a-2;
		for (int b = 0; b < 5; ++b) {
			int yy = b-2;
			vec2 uv2 = uv + (vec2(xx, yy) / screenSize) * coc;
			fragCol += texture(mainTex, uv2) * kernel[a*5 + b];
		}
	}
}
)";
}