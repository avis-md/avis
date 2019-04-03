#pragma once
namespace glsl {
    const char ssaoFrag[] = R"(
#version 330 core

uniform sampler2D normTex;
uniform sampler2D depthTex;
uniform sampler2D noiseTex;
uniform vec2 screenSize;
uniform float radius;
uniform int samples;
uniform mat4x4 _P;
uniform mat4x4 _IP;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize;
	vec3 nrm = texture(normTex, uv).xyz;
	vec3 tan = normalize(cross(nrm, vec3(0, 1, 0)));
	if (length(tan) == 0) tan = normalize(cross(nrm, vec3(0, 0, 1)));
	vec3 bitan = normalize(cross(nrm, tan));
	float z = texture(depthTex, uv).x;

	if (z >= 1) {
		fragCol = vec4(1, 1, 1, 0);
		return;
	}

	vec4 dc = vec4(uv.x*2-1, uv.y*2-1, z*2-1, 1);
	vec4 wPos = _IP*dc;
	wPos *= ceil(1-z) / wPos.w; //world position

	float str = 0;
	float io = texture(noiseTex, uv * screenSize / 16.0).y * (screenSize.x + screenSize.y);
	for (int i = 0; i < samples; ++i) {
		float j = i + io;
		vec3 dw = texture(noiseTex, vec2(j / 16, mod(j, 16)) / 16.0).xyz;
		dw = (dw*2.0) - vec3(1,1,1);
		vec3 wpos2 = wPos.xyz + (nrm*dw.x + tan*dw.y + bitan*dw.z)*radius;
		vec4 spos2 = _P * vec4(wpos2, 1);
		spos2 /= spos2.w;
		spos2.xyz = vec3(spos2.x+1, spos2.y+1, spos2.z+1) * 0.5;
		if (spos2.z < texture(depthTex, spos2.xy).x) {
			str += 1.0 / samples;
		}
	}
	fragCol = vec4(str, str, str, 1);
}
)";
}