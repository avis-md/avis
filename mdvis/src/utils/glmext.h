#pragma once
#include "Engine.h"

class MatFunc {
public:
	static Mat4x4 FromTRS(const Vec3& t, const Quat& r, const Vec3& s);
};
class QuatFunc {
public:
	static Quat Inverse(const Quat&);
	static Vec3 ToEuler(const Quat&);
	static Mat4x4 ToMatrix(const Quat&);
	static Quat FromAxisAngle(Vec3, float);
	static Quat LookAt(const Vec3&);
	static Quat LookAt(const Vec3&, const Vec3&);
};