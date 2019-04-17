#pragma once
namespace glsl {
	const char popupShadFrag[] = R"(
#version 330

uniform vec2 screenSize;
uniform float distance, intensity;
uniform vec4 pos; //screen pixels

out vec4 fragCol;

void main () {
	vec2 uv = gl_FragCoord.xy / screenSize; //screen position xy 0~1
	vec2 d = vec2(abs(gl_FragCoord.x  - (pos.r + 0.5*pos.b)) - 0.5*pos.b, abs(gl_FragCoord.y  - (pos.g + 0.5*pos.a)) - 0.5*pos.a);
	d = max(d/distance, 0);
	
	fragCol = vec4(0, 0, 0, intensity*(1-max(d.x, d.y)));
	//fragCol = vec4(0, 0, 0, abs(gl_FragCoord.y  - (pos.g + 0.5*pos.a)) - 0.5*pos.a);
	
}
)";
}