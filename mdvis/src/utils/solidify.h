#pragma once
#include "Engine.h"

class Solidify {
public:
	static pMesh Do(Vec3* path, uint cnt, float rad, uint dim, Vec3* str = nullptr);
};