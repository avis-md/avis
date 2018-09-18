#pragma once
#include "Engine.h"

class UI3 {
public:
	static void Line(Vec3 p1, Vec3 p2, float w, Vec4 col);
	static void Path(Vec3* path, uint cnt, float w, Vec4 col);
	static void Cube(Vec3 pos, float dx, float dy, float dz, Vec4 col);
	static void Cube(float x1, float x2, float y1, float y2, float z1, float z2, Vec4 col);
	static void Init();

private:
	static GLuint cubeProgIds;
};