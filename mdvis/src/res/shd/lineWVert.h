namespace glsl {
    const char lineWVert[] = R"(
#version 330

uniform samplerBuffer poss;
uniform vec2 width;
uniform mat4 MVP;

void main() {
    int vid = gl_VertexID / 6;
    int vof = int(mod(gl_VertexID, 6));

    vec3 p1 = texelFetch(poss, vid).rgb;
    vec3 p2 = texelFetch(poss, vid + 1).rgb;

    vec4 sp1 = MVP * vec4(p1, 1);
	sp1 /= sp1.w;
    vec4 sp2 = MVP * vec4(p2, 1);
	sp2 /= sp2.w;

    vec2 dir = normalize(sp2.xy-sp1.xy);
    vec2 nrm = vec2(dir.y * width.x, -dir.x * width.y);

    gl_Position = mix(sp1, sp2, mod(vof, 2));
    if (vof > 1 && vof < 5) gl_Position.xy += nrm;
    else gl_Position.xy -= nrm;
}
)";
}