#pragma once
namespace glsl {
	const char marchGeom[] = R"(#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 15) out;

uniform samplerBuffer data;
uniform float val;
uniform ivec3 shp;
uniform vec3 off;

uniform samplerBuffer triBuf;

flat in ivec3[] id;

out vec4 outPos;
out vec4 outNrm;

float get(int x, int y, int z) {
	return texelFetch(data, x * shp.y * shp.z + y * shp.z + z).x;
}

float tget(int x, int y, int z) {
	if ((x < 0 || x >= shp.x)
		|| (y < 0 || y >= shp.y)
		|| (z < 0 || z >= shp.z)) {
		x = clamp(x, 0, shp.x-1);
		y = clamp(y, 0, shp.y-1);
		z = clamp(z, 0, shp.z-1);
	}
	return texelFetch(data, x * shp.y * shp.z + y * shp.z + z).x;
}

vec3 interp(float v1, float v2, vec3 p1, vec3 p2) {
	if (abs(val - v1) < 0.0001f) return p1;
	if (abs(val - v2) < 0.0001f) return p2;
	if (abs(v2 - v1) < 0.0001f) return p1;
	float mul = (val - v1) / (v2 - v1);
	return mix(p1, p2, mul) + off;
}

vec3 _getnrm(ivec3 p, float v) {
	vec3 dif = vec3(tget(p.x-1, p.y, p.z) - v, 0, 0)
		- vec3(tget(p.x+1, p.y, p.z) - v, 0, 0)
		+ vec3(0, tget(p.x, p.y-1, p.z) - v, 0)
		- vec3(0, tget(p.x, p.y+1, p.z) - v, 0)
		+ vec3(0, 0, tget(p.x, p.y, p.z-1) - v)
		- vec3(0, 0, tget(p.x, p.y, p.z+1) - v);
	return normalize(dif);
}

vec3 getnrm(vec3 p, float v) {
	return _getnrm(ivec3(p), v);
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

	vec3 ps[8] = vec3[8](
		vec3(ii,jj,kk+1),
		vec3(ii+1,jj,kk+1),
		vec3(ii+1,jj,kk),
		vec3(ii,jj,kk),
		vec3(ii,jj+1,kk+1),
		vec3(ii+1,jj+1,kk+1),
		vec3(ii+1,jj+1,kk),
		vec3(ii,jj+1,kk)
	);

	float vs[8] = float[8](
		get(ii,jj,kk+1),
		get(ii+1,jj,kk+1),
		get(ii+1,jj,kk),
		get(ii,jj,kk),
		get(ii,jj+1,kk+1),
		get(ii+1,jj+1,kk+1),
		get(ii+1,jj+1,kk),
		get(ii,jj+1,kk)
	);

	int index = 0;
	for (int i = 0; i < 8; i++) {
		if (vs[i] < val) index += (1 << i);
	}

	outPos.w = 1;
	outNrm.w = 0;

	int i2v[24] = int[24](
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	);

	vec3 scl = vec3(1.f / (shp.x-1), 1.f / (shp.y-1), 1.f / (shp.z-1));

	for (int a = 0; a < 5; a++) {
		int a0 = index*15 + a*3;
		int vi = int(texelFetch(triBuf, a0).x);
		if (vi == -1) break;
		float v1 = vs[i2v[vi*2]];
		float v2 = vs[i2v[vi*2+1]];
		vec3 p1 = ps[i2v[vi*2]];
		vec3 p2 = ps[i2v[vi*2+1]];
		emit(interp(v1, v2, p1, p2) * scl, interp(v1, v2, getnrm(p1, v1), getnrm(p2, v2)));

		vi = int(texelFetch(triBuf, a0 + 1).x);
		v1 = vs[i2v[vi*2]];
		v2 = vs[i2v[vi*2+1]];
		p1 = ps[i2v[vi*2]];
		p2 = ps[i2v[vi*2+1]];
		emit(interp(v1, v2, p1, p2) * scl, interp(v1, v2, getnrm(p1, v1), getnrm(p2, v2)));

		vi = int(texelFetch(triBuf, a0 + 2).x);
		v1 = vs[i2v[vi*2]];
		v2 = vs[i2v[vi*2+1]];
		p1 = ps[i2v[vi*2]];
		p2 = ps[i2v[vi*2+1]];
		emit(interp(v1, v2, p1, p2) * scl, interp(v1, v2, getnrm(p1, v1), getnrm(p2, v2)));
		EndPrimitive();
	}
}	
)";}