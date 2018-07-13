#pragma once
namespace glsl {
	const char colorPickerSV[] = R"(
#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
out vec2 UV;
void main(){
	gl_Position.xyz = pos;
	gl_Position.w = 1.0;
	UV = uv;
}
$
#version 330 core
in vec2 UV;
uniform vec3 col;
out vec4 color;
void main(){
	color = vec4(mix(mix(col, vec3(1, 1, 1), UV.x), vec3(0, 0, 0), 1-UV.y), 1);
}
)"; }