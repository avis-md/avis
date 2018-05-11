#pragma once
#include "Engine.h"

class Spline {
public:
	static Vec3 Bezier(const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& p4, float t);

	static void ToSpline(Vec3* pts, uint cnt, uint dim, Vec3* res);
};