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
    const char ssaoFrag2[] = R"(
#version 330 core

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D dtex;
uniform float val;
uniform vec2 screenSize;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize;
	fragCol = mix(texture(tex1, uv), vec4(0,0,0,1),
		min((1-texture(tex2, uv).r) * val, 1) * (1-floor(texture(dtex, uv).r)));
}
)";
}