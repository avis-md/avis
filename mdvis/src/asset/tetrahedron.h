#pragma once
#include "Engine.h"

class Tetrahedron {
public:
	Tetrahedron();

	void Subdivide();
	void ToSphere(float rad);

	std::vector<Vec3> verts, norms;
	std::vector<int> tris;
};