#pragma once
#include "Engine.h"

class CubeMarcher {
public:
	CubeMarcher(Texture3D* tex);

	static void Init();

	void Draw(const Mat4x4& _mvp);

	Vec3 pos, scl;
	Quat rot;

	Texture3D* tex;

private:
	static const int32_t _lookupTable[256*16];
	static GLuint lookupBuf, lookupTex;

	static GLuint prog;
};