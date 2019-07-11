#pragma once
namespace glsl {
	const char parF[] = R"(
#pragma option oriented

uniform mat4 _MV, _P, _IP;

flat in int v2f_id;
in vec3 v2f_pos;
in float v2f_scl;
in float v2f_rad;

uniform vec2 screenSize;
uniform usamplerBuffer id2col;
uniform sampler2D colList;
uniform vec4 gradCols[3];
uniform int colUseGrad;

uniform float orienScl;
uniform samplerBuffer orienX;
uniform samplerBuffer orienY;
uniform samplerBuffer orienZ;

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

void SetColor(int id) {
	int cd = int(texelFetch(id2col, id-1).r);
	if (colUseGrad == 1) {
		outColor.rgb = gradfill(cd/65535.0);
	}
	else {
		outColor.rgb = texture(colList, vec2((mod(cd, 16) + 0.5) / 16.0, ((cd / 16) + 0.5) / 16.0)).rgb;
	}
	outColor.a = 1;
}

void main() {
	float R = v2f_rad * v2f_scl;
#ifdef oriented
	vec4 dir4 = _MV * vec4(
		texelFetch(orienX, v2f_id-1).x,
		texelFetch(orienY, v2f_id-1).x,
		texelFetch(orienZ, v2f_id-1).x, 0);
	vec3 dir = normalize(dir4.xyz);
#endif
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

	vec3 fwd = (wPos - wPos2).xyz;
	vec3 cfwd = (v2f_pos - wPos2.xyz);
	
#ifdef oriented
	float cp2d = dot(-dir, fwd);
	wPos2.xyz -= dir * cp2d / orienScl;
	float vp2d = dot(dir, v2f_pos - wPos.xyz);
	vec3 v2fp2 = v2f_pos - dir * vp2d * (1 - 1.0 / orienScl);
	fwd = (wPos - wPos2).xyz;
	cfwd = v2fp2 - wPos2.xyz;
#endif

	float C2 = length2(cfwd);
	fwd = normalize(fwd);
	//cfwd = normalize(cfwd);
	float cs = dot(fwd, cfwd);

	float isq = cs*cs - C2 + R*R;

	if (cs < 0 || isq < 0) {
		discard;
	}
	else {
		float vl = (cs - sqrt(isq));
		if (vl < 0) {
			discard;
		}
		vec3 s = wPos2.xyz + fwd * vl;

		vec4 ndc2 = _P * vec4(s, 1);
		ndc2 /= ndc2.w;
		gl_FragDepth = ndc2.z * 0.5 + 0.5;
		SetColor(v2f_id);
		outId = uvec4(v2f_id, 0, 0, 0);

		vec3 nt = (s - v2f_pos);
#ifdef oriented
		float nt2d = dot(nt, dir);
		outNormal.xyz = normalize(nt - dir * nt2d * (1 - 1.0 / orienScl));
#else
		outNormal.xyz = normalize(nt);
#endif
	}
}
)";
}