#pragma once
namespace glsl {
	const char colorPickerSV[] = R"(
#version 330 core
in vec2 UV;
uniform vec3 col;
out vec4 color;
void main(){
	color = vec4(mix(mix(col, vec3(1, 1, 1), UV.x), vec3(0, 0, 0), 1-UV.y), 1);
}
)"; }