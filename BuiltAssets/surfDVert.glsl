#version 330 core

layout(location=0) in vec4 pos;
layout(location=1) in vec4 nrm;

uniform mat4 _MVP;

void main(){
	gl_Position = _MVP * pos;
}