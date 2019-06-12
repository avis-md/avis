#pragma once
namespace glsl {
	const char surfDVert[] = R"(
#version 330 core

layout(location=0) in vec4 pos;
layout(location=1) in vec4 nrm;

uniform mat4 _MV;
uniform mat4 _MVP;
uniform vec3 bbox1;
uniform vec3 bbox2;

out vec3 v2f_nrm;

void main(){
	vec4 pos2 = vec4(
		mix(bbox1.x, bbox2.x, pos.x),
		mix(bbox1.y, bbox2.y, pos.y),
		mix(bbox1.z, bbox2.z, pos.z),
	1);
	gl_Position = _MVP * pos2;
	v2f_nrm = (_MV * nrm).xyz;
}
)";
	const char surfDFrag[] = R"(
#version 330 core

layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;

in vec3 v2f_nrm;

void main() {
    outColor = vec4(0, 0.7, 1, 1);
    outId = uvec4(0, 0, 0, 0);
    outNormal.xyz = normalize(v2f_nrm * sign(-v2f_nrm.z));
    outNormal.w = 0;
}
)";
}