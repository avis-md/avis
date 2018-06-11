#pragma once
#include "Engine.h"

class plt {
public:
	static void plot(float x, float y, float w, float h, float* dx, float* dy, uint cnt, Font* font = nullptr, uint sz = 0, Vec4 col = black());
};