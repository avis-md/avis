#pragma once
#include "Engine.h"

class Arrow {
public:
	Arrow(float w1, float h1, float w2, float h2, int res);

	std::vector<Vec3> verts, norms;
	std::vector<int> tris;
};