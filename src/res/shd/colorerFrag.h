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
	const char colererFrag[] = R"(
#version 330

uniform usampler2D idTex;
uniform vec2 screenSize;
uniform samplerBuffer id2col;
uniform sampler2D colList;
uniform uint doid;
uniform int usegrad;
uniform vec4 gradcols[3];
uniform vec4 tint;

layout (location = 0) out vec4 fragCol;

vec3 gradfill(float f) {
	return (gradcols[0].rgb * clamp(2 - 4 * f, 0, 1)
		+ gradcols[1].rgb * clamp(2 - abs(2 - 4 * f), 0, 1)
		+ gradcols[2].rgb * clamp(4 * f - 2, 0, 1));
}

void main () {
	uvec4 tx = texture(idTex, gl_FragCoord.xy / screenSize);
	if (tx.y > 65535U) { //protein
		float v = float(tx.x);
		fragCol = vec4(gradfill(1 - (v / 65535)), 1);
		return;
	}
	int id = int(tx.x);
	if (id == 0) {
		fragCol = vec4(0.5, 0.5, 0.5, 1);
		return;
	}
	float cdf = texelFetch(id2col, id-1).r;
	int cd = int(cdf * 255);
	if (doid != 0U) {
		if (doid == tx.y) fragCol = tint;
		else fragCol = vec4(0, 0, 0, 0);	
		return;
	}
	else if (usegrad == 1) fragCol.rgb = gradfill(cd / 255.0);
	else {
		fragCol = texture(colList, vec2((mod(cd, 16) + 0.5) / 16.0, ((cd / 16) + 0.5) / 16.0));
		if (tx.y == 1U)
			fragCol = mix(fragCol, tint, tint.a);
	}
	fragCol.a = 1;
}
)";
}