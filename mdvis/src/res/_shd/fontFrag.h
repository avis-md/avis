#pragma once
namespace glsl {
	const char fontFrag[] = R"(
#version 330
in vec2 UV;
uniform sampler2D sampler;
uniform vec4 col;
out vec4 color;
void main() {
	color = vec4(1, 1, 1, texture(sampler, UV).r)*col;
}
)";
}