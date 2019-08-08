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
    const char parConV[] = R"(
const float PI = 3.14159;
const float fov = 60 * PI / 180;

uniform mat4 _MV, _P;
uniform vec3 camPos;
uniform vec3 camFwd;

uniform vec2 screenSize;

uniform samplerBuffer posTex;
uniform usamplerBuffer connTex;
uniform samplerBuffer radTex;
uniform float radScl;
uniform float orthoSz;
uniform float spriteScl;
uniform float tubesize;

uniform vec3 bbox;
uniform ivec3 imgCnt;
uniform ivec3 imgOff;

layout (std140) uniform clipping {
    vec4 clip_planes[6];
};
bool clipped(vec3 pos) {
    for (int a = 0; a < 6; ++a)  {
        if (dot(pos, clip_planes[a].xyz) > clip_planes[a].w)
            return true;
    }
    return false;
}

flat out int v2f_id1;
flat out int v2f_id2;
out vec3 v2f_wpos1;
out vec3 v2f_wpos2;
out float v2f_scl;

float len2(vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

int is_cw(vec2 v1, vec2 v2) {
	return (v1.x*v2.y-v1.y*v2.x) > 0 ? 1 : 0;
}

float maxrad(float rad, vec3 wpos) {
	float psz = 0;
	if (orthoSz < 0) {
		vec3 wdir = wpos - camPos;
		float d = length(wdir);
		float ca = dot(wdir / d, camFwd);
		float z = d * ca;
		
		float th1 = acos(ca);
		if (th1 > fov) {		
			return -1.0;
		}
		float ym = z * tan(fov/2);
		float xm = ym * screenSize.x / screenSize.y;

		float th2 = asin(rad / d);

		float rl = tan(th1 + th2) * z;
		rl = rl - sqrt(d * d - z * z);

		return spriteScl * rl * min(screenSize.x / xm, screenSize.y / ym);
	}
	else {
		return spriteScl * screenSize.x * rad / orthoSz;
	}
}

void main(){
	float maxDst2 = 3;

	int pid = gl_VertexID / 12;
	int vid = int(mod(gl_VertexID, 12));

    uint id1 = texelFetch(connTex, pid * 2).r;
    uint id2 = texelFetch(connTex, pid * 2 + 1).r;
    v2f_id1 = int(id1) + 1;
    v2f_id2 = int(id2) + 1;
	vec3 pos1 = texelFetch(posTex, int(id1)).rgb;
	vec3 pos2 = texelFetch(posTex, int(id2)).rgb;

	int iz = int(mod(gl_InstanceID, imgCnt.z));
	int gi2 = int(gl_InstanceID / imgCnt.z);
	int iy = int(mod(gi2, imgCnt.y));
	int ix = int(gi2 / imgCnt.y);
	pos1 += bbox * (vec3(ix, iy, iz) - imgOff);
	pos2 += bbox * (vec3(ix, iy, iz) - imgOff);

	if (len2(pos2 - pos1) > maxDst2) {
		gl_Position = vec4(-1, -1, -1, 1);
		return;
	}
	if (texelFetch(radTex, int(id1)).r < 0 || texelFetch(radTex, int(id2)).r < 0) {
		gl_Position = vec4(-1, -1, -1, 1);
		return;
	}
	vec4 unitVec = _MV*vec4(1, 0, 0, 0);
	v2f_scl = length(unitVec) * radScl;
	
	vec4 wpos = _MV*vec4(pos1, 1);
    v2f_wpos1 = wpos.xyz / wpos.w;
    vec4 spos = _P*wpos;
	vec2 spos1 = spos.xy / spos.w;
	
	float rad = tubesize * v2f_scl;
	float r1 = maxrad(rad, v2f_wpos1);
	if (r1 < 0) {
		gl_Position = vec4(-1, -1, -1, 1);
		return;
	}

	wpos = _MV*vec4(pos2, 1);
    v2f_wpos2 = wpos.xyz / wpos.w;
	if (clipped(v2f_wpos1) || clipped(v2f_wpos2)) {
		gl_Position = vec4(-1, -1, -1, 1);
		return;
	}

    spos = _P*wpos;
	vec2 spos2 = spos.xy / spos.w;
	
	float r2 = maxrad(rad, v2f_wpos2);
	if (r2 < 0) {
		gl_Position = vec4(-1, -1, -1, 1);
		return;
	}

	vec2 pts[6];
	int ids[12] = int[]( 0, 1, 5, 0, 5, 2, 0, 2, 3, 0, 3, 4 );

	vec2 cd = spos2 - spos1;
	cd *= screenSize / 2;
	float adr = abs(r2 - r1);
	if (adr > abs(cd.x) && adr > abs(cd.y)) {
		gl_Position = vec4(-1, -1, -1, 1);
		return;
	}
	vec2 n1 = normalize(-cd);
	vec2 td1, td2;
	if (abs(n1.x) > abs(n1.y)) {
		td1.x = td2.x = sign(n1.x);
		td1.y = 1;
		td2.y = -1;
 	}
	else {
		td1.y = td2.y = sign(n1.y);
		td1.x = 1;
		td2.x = -1;	
	}
	td1 *= spriteScl * 2 / screenSize;
	td2 *= spriteScl * 2 / screenSize;
	pts[0] = spos1 + td1*r1;
	pts[1] = spos1 + td2*r1;
	pts[2] = spos2 - td1*r2;
	pts[3] = spos2 - td2*r2;

	vec2 pd1 = spos1 - td2*r1;
	vec2 pd2 = spos2 + td1*r2;
	pts[4] = (is_cw(pd1-pts[0], pts[3]-pts[0]) == is_cw(pd1-pts[0], pts[1]-pts[0]))? pd1 : pd2;

	pd1 = spos1 - td1*r1;
	pd2 = spos2 + td2*r2;
	pts[5] = (is_cw(pd1-pts[1], pts[0]-pts[1]) == is_cw(pd1-pts[1], pts[2]-pts[1]))? pd1 : pd2;

	gl_Position.xy = pts[ids[vid]];
	gl_Position.z = 0;
	gl_Position.w = 1;
}
)";
}