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
	const char voxelFrag[] = R"(
#version 330
in vec3 v2f_uvw;
in vec3 v2f_wpos;

uniform vec2 screenSize;
uniform mat4 _IP;
uniform sampler3D tex;
out vec4 outColor;
void main()
{
	vec2 uv = gl_FragCoord.xy / screenSize;
	vec4 cc = vec4(uv.x*2-1, uv.y*2-1, -1, 1);
	vec4 cp = _IP*cc;
	vec3 camPos = (cp / cp.w).xyz;
	
	vec3 eye = v2f_wpos - camPos;
	eye = normalize(eye);
	
	outColor = vec4(0, 0, 0, 1);
	for (int i = 0; i < 100; ++i) {
		vec3 uvw = v2f_uvw + eye * i / 100.0;
		if (uvw.x < 0 || uvw.x > 1 || uvw.y < 0 || uvw.y > 1 || uvw.y < 0 || uvw.y > 1)
			return;
		else
			outColor.rgb += texture(tex, uvw).rgb / 100.0;
	}
	
	outColor.rgb = texture(tex, v2f_uvw).xyz;
}
)";
}