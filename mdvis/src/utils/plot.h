#pragma once
#include "Engine.h"

class plt {
public:
	struct remapdata {
		int type, selId;
		std::vector<Vec2> pts;
	};

	static void plot(float x, float y, float w, float h, float* dx, float* dy, uint cnt, Font* font = nullptr, uint sz = 0, Vec4 col = black());
	static void remap(float x, float y, float w, float h, remapdata& data);
};