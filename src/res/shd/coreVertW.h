#pragma once
namespace glsl {
    const char coreVertW[] = R"(
#version 330 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
uniform mat4 MVP;
out vec2 UV;
void main(){
	gl_Position = MVP * vec4(pos, 1);
	//gl_Position /= gl_Position.w;
	UV = uv;
}
)";
}