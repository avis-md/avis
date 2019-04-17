#version 330 core

uniform mat4 _MV, _P;
uniform samplerBuffer posTex;
uniform usamplerBuffer connTex;
uniform vec2 rad;

flat out uint v2f_id1;
flat out uint v2f_id2;
out float v2f_u;

float len2(vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

void main(){
	float maxDst2 = 1;

	int vid = (gl_VertexID / 6) * 2;
    uint id1 = texelFetch(connTex, vid).r;
    uint id2 = texelFetch(connTex, vid + 1).r;
    v2f_id1 = id1 + 1U;
    v2f_id2 = id2 + 1U;
	v2f_u = mod(gl_VertexID, 2);
    vec3 pos1 = texelFetch(posTex, int(id1)).rgb;
    vec3 pos2 = texelFetch(posTex, int(id2)).rgb;
    if (len2(pos2-pos1) > maxDst2) {
        gl_Position = vec4(-1, -1, -1, 1);
        return;
    }
    vec4 spos1 = _P*_MV*vec4(pos1, 1);
    spos1 /= spos1.w;
    vec4 spos2 = _P*_MV*vec4(pos2, 1);
    spos2 /= spos2.w;
    vec3 dir = spos2.xyz - spos1.xyz;
    gl_Position = mix(spos1, spos2, v2f_u);
    if ((spos1.x > -1 && spos1.x < 1 && spos1.y > -1 && spos1.y < 1) ||
        (spos2.x > -1 && spos2.x < 1 && spos2.y > -1 && spos2.y < 1)) {
        vec2 dir2 = normalize(cross(vec3(dir.xy, 0), vec3(0, 0, 1)).xy);
        int qid = int(mod(gl_VertexID, 6));
        if (qid == 0 || qid > 3)
            gl_Position.xy -= dir2 * rad;
        else
            gl_Position.xy += dir2 * rad;
    }
}