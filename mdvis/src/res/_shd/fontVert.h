#pragma once
namespace glsl {
	const char fontVert[] = R"(
#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in float c;
out vec2 UV;
void main(){
	gl_Position.xyz = pos*2 - vec3(1,1,0);
	gl_Position.w = 1;
	vec2 uv1 = vec2(mod(c, 16), floor(c/16));
	UV = (uv1 + uv)/16;
}
)";
}