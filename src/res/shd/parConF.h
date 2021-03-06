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
    const char parConF[] = R"(
flat in int v2f_id1;
flat in int v2f_id2;
in vec3 v2f_wpos1;
in vec3 v2f_wpos2;
in float v2f_scl;

uniform mat4 _P;
uniform mat4 _IP;
uniform vec2 screenSize;
uniform uint id2;
uniform isamplerBuffer id2col;
uniform sampler2D colList;
uniform vec4 gradCols[3];
uniform int colUseGrad;
uniform int usegrad;
uniform vec4 onecol;
uniform float tubesize;

layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;

float length2 (vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

vec3 gradfill(float f) {
	return (gradCols[0].rgb * clamp(2 - 4 * f, 0, 1)
		+ gradCols[1].rgb * clamp(2 - abs(2 - 4 * f), 0, 1)
		+ gradCols[2].rgb * clamp(4 * f - 2, 0, 1));
}

void SetColor(int id1, int id2, float f) {
    int cd1 = int(texelFetch(id2col, id1-1).r);
    int cd2 = int(texelFetch(id2col, id2-1).r);
    vec3 cl1 = (colUseGrad == 1)? gradfill(cd1 / 65535.0f)
        : texture(colList, vec2((mod(cd1, 16) + 0.5) / 16.0, ((cd1 / 16) + 0.5) / 16.0)).rgb;
    vec3 cl2 = (colUseGrad == 1)? gradfill(cd2 / 65535.0f)
        : texture(colList, vec2((mod(cd2, 16) + 0.5) / 16.0, ((cd2 / 16) + 0.5) / 16.0)).rgb;
    outColor.rgb = mix(mix(cl1, cl2, f), onecol.rgb, onecol.a);
    outColor.a = 1;
}

vec2 lli(vec3 p1, vec3 p2, vec3 d1, vec3 d2) {
	float dab = dot(d1, d2);
	if (dab >= 1 || dab <= -1) return vec2(0, 0);
	
	vec3 c = p1 - p2;
	float dac = dot(d1, c);
	float dbc = dot(d2, c);
	
	float num = dbc * dab - dac;
	float div = 1 - dab * dab;
	float resx = num / div;
	float resy = dbc + resx * dab;
	return vec2(resx, resy);
}

float len2(vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

void main()
{
    vec3 td = normalize(v2f_wpos2 - v2f_wpos1);
    float lm = length(v2f_wpos2 - v2f_wpos1);
    float L = length(v2f_wpos2 - v2f_wpos1);
    float r = tubesize * 2 * v2f_scl;
    //*
    vec4 ndc = vec4(
        (gl_FragCoord.x / screenSize.x - 0.5) * 2.0,
        (gl_FragCoord.y / screenSize.y - 0.5) * 2.0,
        (gl_FragCoord.z - 0.5) * 2.0,
        1.0);

    vec4 wPos = _IP * ndc;
    wPos /= wPos.w;

    ndc.z = -1;
    vec4 wPos2 = _IP * ndc;
    wPos2 /= wPos2.w;

    vec3 fwd = normalize((wPos - wPos2).xyz);

    vec2 lts = lli(wPos2.xyz, v2f_wpos1, fwd, td);
    vec3 p1 = v2f_wpos1 + lts.y * td;
    vec3 p2 = wPos2.xyz + lts.x * fwd;

    float lp12 = length(p1 - p2);
    if (lp12 > r) {
        discard;
    }
    else {
        float p232 = r*r - lp12*lp12;
        float p23 = sqrt(p232);
        vec3 d1 = cross(td, normalize(p2 - p1));
        
        if (lp12 < 0.0001) {
            vec3 td1 = cross(fwd, td);
            d1 = normalize(cross(td, td1));
        }
        
        float rtd = dot(fwd, td);
        float phi = 1;
        if (rtd < 0.9999 && rtd > -0.9999) {
            phi = dot(fwd, d1);
            if (phi < 0) phi *= -1;
        }
        float lf = p23 / phi;
        
        vec3 pf = p2 - fwd * lf;
        
        float l3f2 = lf * lf - p232;
        float l3f = 0;
        if (l3f2 > 0)
            l3f = sqrt(l3f2);
        
        vec3 td2 = td;
        if (dot(td2, fwd) > 0) td2 *= -1;
        vec3 cf = p1 + td2 * l3f;
        
        float lc2 = len2(cf - v2f_wpos1);
        if ((dot(cf - v2f_wpos1, td) < 0) || (lc2 > lm*lm)) {
            discard;
        }
        int id = (sqrt(lc2)/lm > 0.5) ? v2f_id2 : v2f_id1;
        
        vec4 ndc2 = _P * vec4(pf, 1);
        ndc2 /= ndc2.w;
        if (ndc2.z < -1) discard;
        gl_FragDepth = ndc2.z * 0.5 + 0.5;
        
        float et = sqrt(lc2)/lm;

        SetColor(v2f_id1, v2f_id2, mix(round(et), et, usegrad));
        outId = uvec4(id, id2, 0, 0);
        outNormal.xyz = normalize(pf - cf);
    }
}
)";
}