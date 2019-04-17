#pragma once
namespace glsl {
    const char coreFragSky[] = R"(
#version 330 core
in vec2 UV;
uniform sampler2D sampler;
uniform vec2 dir;
uniform float length;
out vec4 color;
void main(){
	float ay = asin((UV.y) / length);
	float l2 = length*cos(ay);
	float ax = asin((dir.x + UV.x) / l2);
	color = textureLod(sampler, vec2((dir.x + ax / 3.14159)*sin(dir.y + ay / 3.14159) + 0.5, (dir.y + ay / 3.14159)), 0);
	color.a = 1;
}
)";
}