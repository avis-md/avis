#version 330 core

in vec3 v2f_nrm;

uniform vec4 color;

layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;

void main() {
    outColor = color;
    outId = uvec4(0, 0, 0, 0);
    outNormal.xyz = normalize(v2f_nrm);
    outNormal.w = 1;
}