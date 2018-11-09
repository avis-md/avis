#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 nrm;

uniform mat4 _MVP;

out vec3 v2f_n;

void main(){
	gl_Position = _MVP*pos;
}