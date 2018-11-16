#pragma once
#include "Engine.h"

class Mesh : public RefCnt<Mesh> {
public:
	Mesh();
	Mesh(int vsz, Vec3* pos, Vec3* norm, int tsz, int* tri, bool sv = false);
	~Mesh();

	std::vector<Vec3> vertices;
	std::vector<Vec3> normals;
	std::vector<int> triangles;

	uint vertCount, triCount;

	GLuint vao, vbos[2], veo;
};
