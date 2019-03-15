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