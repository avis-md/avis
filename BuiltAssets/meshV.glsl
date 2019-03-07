#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 nrm;

uniform mat4 _MV, _P;

out vec3 v2f_nrm;

void main(){
	vec4 wpos = _MV*vec4(pos, 1);
	wpos /= wpos.w;
	/*if (clipped(wpos.xyz)) {
		gl_PointSize = 0;
		return;
	}*/
	gl_Position = _P*wpos;
	v2f_nrm = (_MV*vec4(nrm, 0)).xyz;
}