#pragma once
#include "ChokoLait.h"

class Shadows {
public:
	static Camera* cam;

	static bool show;
	static Vec3 pos;
	static float str;
	static float rw, rz;
	static Quat rot;
	static float dst, dst2;
	static float box[6];
	static Mat4x4 _p, _ip;

	static GLuint _fbo, _dtex;

	static void Init();
	static void UpdateBox();
	static void Rerender(), Reblit();
	static float DrawMenu(float off);
};