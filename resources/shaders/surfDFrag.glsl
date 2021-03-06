#version 330 core

layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;

in vec3 v2f_nrm;

void main() {
    outColor = vec4(1, 0, 0, 1);
    outId = uvec4(0, 0, 0, 0);
    outNormal.xyz = normalize(v2f_nrm);
    outNormal.w = 0;
}