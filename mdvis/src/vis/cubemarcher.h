#pragma once
#include "Engine.h"

class CubeMarcher {
public:
	CubeMarcher(Texture3D* tex);

	static void Init();

	void Draw(const Mat4x4& _mvp);

	static GLuint prog;

	Vec3 pos, scl;
	Quat rot;

	Texture3D* tex;
};