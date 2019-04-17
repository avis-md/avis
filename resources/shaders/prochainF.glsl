#version 330 core

in vec3 v2f_normal;
in float v2f_chpos;

uniform int proId;

layout (location=0) out ivec4 outId; //rgba
layout (location=1) out vec4 outNormal; //xyz []
layout (location=2) out vec4 outSpec; //spec gloss
layout (location=3) out vec4 outEmi; //emi occlu

void Output(ivec4 col, vec3 norm, vec3 spec, float gloss, vec4 emi) {
	outId = col;
	outNormal.rgb = normalize(norm);
	outNormal.a = 0;
	outSpec.rgb = spec;
	outSpec.a = gloss;
	outEmi = emi;
}

void main()
{
	Output(ivec4(int(v2f_chpos*65535),proId + 65536,0,0), v2f_normal.xyz, vec3(0.2, 0.2, 1), 1, vec4(0, 0, 0, 0));
}