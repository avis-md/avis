#pragma once
#include "Engine.h"

class Tetrahedron : public _Mesh {
public:
	Tetrahedron();

	void Subdivide();
	void ToSphere(float rad);
};