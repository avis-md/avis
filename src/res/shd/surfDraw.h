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
	const char surfDVert[] = R"(
#pragma option use_clip

layout(location=0) in vec3 pos;
layout(location=1) in vec3 nrm;

uniform mat4 _MV;
uniform mat4 _MVP;
uniform vec3 bbox1;
uniform vec3 bbox2;
uniform float direction;

layout (std140) uniform clipping {
    vec4 clip_planes[6];
};

out vec3 v2f_pos;
out vec3 v2f_nrm;

bool clipped(vec3 pos) {
    for (int a = 0; a < 6; ++a)  {
        if (dot(pos, clip_planes[a].xyz) > clip_planes[a].w)
            return true;
    }
    return false;
}

void main(){
	vec4 pos2 = vec4(
		mix(bbox1.x, bbox2.x, pos.x),
		mix(bbox1.y, bbox2.y, pos.y),
		mix(bbox1.z, bbox2.z, pos.z),
	1);
	gl_Position = _MVP * pos2;
	vec4 wpos = _MV * pos2;
	wpos /= wpos.w;

	v2f_pos = wpos.xyz;
	v2f_nrm = direction * (_MV * vec4(nrm, 0)).xyz;
}
)";
	const char surfDFrag[] = R"(
#pragma option use_clip

in vec3 v2f_pos;
in vec3 v2f_nrm;

layout (std140) uniform clipping {
    vec4 clip_planes[6];
};

layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;

bool clipped(vec3 pos) {
    for (int a = 0; a < 6; ++a)  {
        if (dot(pos, clip_planes[a].xyz) > clip_planes[a].w)
            return true;
    }
    return false;
}

void main() {
#ifdef use_clip
	if (clipped(v2f_pos)) {
		discard;
	}
#endif

    outColor = vec4(0.1, 0.7, 1, 1);
    outId = uvec4(0, 0, 0, 0);
    outNormal.xyz = normalize(v2f_nrm);// * sign(-v2f_nrm.z));
    outNormal.w = 0;
}
)";
}