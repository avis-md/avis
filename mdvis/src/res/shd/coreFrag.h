#pragma once
namespace glsl {
    const char coreFrag[] = R"(
#version 330 core
in vec2 UV;
uniform sampler2D sampler;
uniform vec4 col;
uniform float level;
out vec4 color;
void main(){
	if (level < 0)
		color = texture(sampler, UV)*col;
	else
		color = textureLod(sampler, UV, level)*col;
}
)";
}