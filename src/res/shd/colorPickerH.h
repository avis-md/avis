#pragma once
namespace glsl {
	const char colorPickerH[] = R"(
#version 330 core
in vec2 UV;
out vec4 color;
void main(){
	float hue = 6 - UV.y*6;
	vec4 v;

	v.r = clamp(abs(hue - 3) - 1, 0, 1);

	v.g = 1 - clamp(abs(hue - 2) - 1, 0, 1);

	v.b = 1 - clamp(abs(hue - 4) - 1, 0, 1);
	v.a = 1;
	color = v;
}
)"; }