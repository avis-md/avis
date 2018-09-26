#pragma once
#include "Engine.h"

class Mesh {
public:
	Mesh(int vsz, Vec3* pos, Vec3* norm, int tsz, int* tri, bool sv = false);

	std::vector<Vec3> vertices;
	std::vector<Vec3> normals;
	std::vector<int> triangles;

	uint vertCount, triCount;

	friend class Engine;
	_allowshared(Mesh);
protected:
	GLuint vao, vbos[2], veo;
};
