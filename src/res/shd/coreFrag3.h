#pragma once
namespace glsl {
	const char coreFrag3[] = R"(
#version 330 core
uniform vec4 col;
layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;
void main(){
	outColor = col;
	outId = uvec4(0,0,0,0);
	outNormal = vec4(0,0,0,0);
}
)";
}