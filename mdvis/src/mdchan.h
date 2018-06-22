#pragma once
#include "Engine.h"

class MdChan {
public:
	static Texture* texs[4];
	static float blink, t4, t4d;

	static void Init();

	static void Draw(Vec2 pos);
};