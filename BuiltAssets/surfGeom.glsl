#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

uniform samplerBuffer data;
uniform float val;
uniform ivec3 shp;

uniform samplerBuffer triBuf;

flat in ivec3[] id;

out vec4 outPos;
out vec4 outNrm;

float get(int x, int y, int z) {
	return texelFetch(data, x * shp.y * shp.z + y * shp.z + z).x;
}

vec3 interp(float v1, float v2, vec3 p1, vec3 p2) {
	if (abs(val - v1) < 0.0001f) return p1;
	if (abs(val - v2) < 0.0001f) return p2;
	if (abs(v2 - v1) < 0.0001f) return p1;
	float mul = (val - v1) / (v2 - v1);
	return mix(p1, p2, mul);
}

void emit(vec3 p, vec3 n) {
	outPos.xyz = p;
	outNrm.xyz = n;
	EmitVertex();
}

void main() {
	int ii = id[0].x;
	int jj = id[0].y;
	int kk = id[0].z;

	vec3 p0 = vec3(ii,jj,kk+1);
	vec3 p1 = vec3(ii+1,jj,kk+1);
	vec3 p2 = vec3(ii+1,jj,kk);
	vec3 p3 = vec3(ii,jj,kk);
	vec3 p4 = vec3(ii,jj+1,kk+1);
	vec3 p5 = vec3(ii+1,jj+1,kk+1);
	vec3 p6 = vec3(ii+1,jj+1,kk);
	vec3 p7 = vec3(ii,jj+1,kk);

	float v0 = get(ii,jj,kk+1);
	float v1 = get(ii+1,jj,kk+1);
	float v2 = get(ii+1,jj,kk);
	float v3 = get(ii,jj,kk);
	float v4 = get(ii,jj+1,kk+1);
	float v5 = get(ii+1,jj+1,kk+1);
	float v6 = get(ii+1,jj+1,kk);
	float v7 = get(ii,jj+1,kk);

	int index = 0;
	if (v0 < val) index += 1;
	if (v1 < val) index += 2;
	if (v2 < val) index += 4;
	if (v3 < val) index += 8;
	if (v4 < val) index += 16;
	if (v5 < val) index += 32;
	if (v6 < val) index += 64;
	if (v7 < val) index += 128;

	outPos.w = 1;
	outNrm.w = 0;

	vec3 vpos[12] = vec3[12] (
		interp(v0, v1, p0, p1),
		interp(v1, v2, p1, p2),
		interp(v2, v3, p2, p3),
		interp(v3, v0, p3, p0),
		interp(v4, v5, p4, p5),
		interp(v5, v6, p5, p6),
		interp(v6, v7, p6, p7),
		interp(v7, v4, p7, p4),
		interp(v0, v4, p0, p4),
		interp(v1, v5, p1, p5),
		interp(v2, v6, p2, p6),
		interp(v3, v7, p3, p7)
	);
	for (int a = 0; a < 5; a++) {
		int a0 = index*15 + a*3;
		int vi = int(texelFetch(triBuf, a0).x);
		if (vi == -1) break;
		emit(vpos[vi], vec3(0, 0, 0));
		vi = int(texelFetch(triBuf, a0 + 1).x);
		emit(vpos[vi], vec3(0, 0, 0));
		vi = int(texelFetch(triBuf, a0 + 2).x);
		emit(vpos[vi], vec3(0, 0, 0));
		EndPrimitive();
	}
}