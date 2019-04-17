#pragma once
namespace glsl {
    const char glowFrag2[] = R"(
#version 330 core

in vec2 uv;

uniform sampler2D mainTex;
uniform sampler2D glowTex;
uniform float str;
uniform vec2 screenSize;

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize;
	
	fragCol = texture(mainTex, uv) + texture(glowTex, uv) * str;
}
)";
}