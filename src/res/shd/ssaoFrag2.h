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