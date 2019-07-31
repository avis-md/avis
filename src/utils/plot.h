// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "Engine.h"

class plt {
public:
	struct remapdata {
		int type, selId;
		std::vector<Vec2> pts;

		float Eval(float f);
	};

	static void plot(float x, float y, float w, float h, float* dx, float* dy, uint cnt, Font* font = nullptr, uint sz = 0, Vec4 col = black());
	static void plot(float x, float y, float w, float h, float* dx, float** dy, uint cnt, uint cnt2, Font* font = nullptr, uint sz = 0, Vec4 col = black());
	static void remap(float x, float y, float w, float h, remapdata& data);
};