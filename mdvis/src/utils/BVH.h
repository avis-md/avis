#pragma once
#include "Engine.h"

class BVH {
public:
	struct Ball {
		Vec3 orig;
		float rad;
	};
	struct Node {
		BBox box; //leaf if x0 > x1, val2 if y0 > 0
		uint32_t val1;
		uint32_t val2;
	};

	static void Calc(Ball* objs, uint cnt, Node*& res, uint& resCnt, BBox box);
};