#pragma once
#include "Engine.h"

//TODO: enable copy without pointers
class Mesh {
public:
	Mesh(int vsz, Vec3* pos, Vec3* norm, int tsz, int* tri, bool sv = false);

	std::vector<Vec3> vertices;
	std::vector<Vec3> normals;
	std::vector<int> triangles;

	uint vertCount, triCount;

	GLuint vao, vbos[2], veo;
};
