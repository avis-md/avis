#pragma once
#include "Engine.h"

class Solidify {
public:
	static void GetTangents(Vec3* path, uint cnt, Vec3* tans);
	static pMesh Do(Vec3* path, uint cnt, float rad, uint dim, Vec3* str = nullptr);
};