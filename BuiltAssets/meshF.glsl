#version 330 core

in vec3 v2f_n;

uniform vec4 col;
uniform uint id;

layout (location=0) out vec4 outColor;
layout (location=1) out uvec4 outId;
layout (location=2) out vec4 outNormal;

void main() {
    outColor = col;
    outId.x = 0;
    outId.y = id;
    outNormal = normalize(v2f_n);
}