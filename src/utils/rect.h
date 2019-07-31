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

class Rect {
public:
	Rect() : x(0), y(0), w(1), h(1) {}
	Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
	Rect(Vec4 v) : x(v.r), y(v.g), w(v.b), h(v.a) {}
	float x, y, w, h;

	/*! Check if v is inside this rect.
	*/
	bool Inside(const Vec2& v);
	/*! Returns a new Rect covered by both this rect and r2
	*/
	Rect Intersection(const Rect& r2);
};