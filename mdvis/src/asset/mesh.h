#pragma once
#include "Engine.h"

class _Mesh {
public:
	std::vector<Vec3> vertices;
	std::vector<Vec3> normals;
	std::vector<int> triangles;

	uint vertCount, triCount;
};

class Mesh : public _Mesh, public RefCnt {
public:
	Mesh();
	Mesh(int vsz, Vec3* pos, Vec3* norm, int tsz, int* tri, bool sv = false);
	~Mesh();

	GLuint vao, vbos[2], veo;

	void DestroyRef() override;
};
