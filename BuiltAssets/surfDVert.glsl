#version 330 core

layout(location=0) in vec4 pos;
layout(location=1) in vec4 nrm;

uniform mat4 _MV;
uniform mat4 _MVP;

out vec3 v2f_nrm;

void main(){
	gl_Position = _MVP * pos;
	v2f_nrm = (_MV * nrm).xyz;
}