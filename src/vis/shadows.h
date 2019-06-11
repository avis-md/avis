#pragma once
#include "ChokoLait.h"

class Shadows {
public:
	static pCamera cam;

	static bool show;
	static byte quality;
	static Vec3 pos, cpos;
	static float str, bias;
	static float rw, rz;
	static Quat rot;
	static float dst, dst2;
	static float box[6];
	static uint _sz;
	static Mat4x4 _p, _ip;

	static bool isPass;

	static GLuint _fbo, _dtex;

	static Shader _prog;;

	static void Init();
	static void UpdateBox();
	static void Rerender(), Reblit();
	static float DrawMenu(float off);

	static void Serialize(XmlNode* nd);

	static void Deserialize(XmlNode* nd);
};