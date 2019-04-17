#pragma once
#include "Engine.h"

struct BBox {
	BBox() : x0(0), y0(0), z0(0), x1(0), y1(0), z1(0) {}
	BBox(float, float, float, float, float, float);

	float x0, y0, z0, x1, y1, z1;
};