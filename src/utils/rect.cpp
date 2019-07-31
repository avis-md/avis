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

#include "Engine.h"

bool Rect::Inside(const Vec2& v) {
	return ((w > 0) ? (v.x > x && v.x < (x + w)) : (v.x >(x + w) && v.x < x)) && ((h > 0) ? (v.y > y && v.y < (y + h)) : (v.y >(y + h) && v.y < y));
}

Rect Rect::Intersection(const Rect& r2) {
	float ox = std::max(x, r2.x);
	float oy = std::max(y, r2.y);
	float p2x = std::min(x + w, r2.x + r2.w);
	float p2y = std::min(y + h, r2.y + r2.h);
	return Rect(ox, oy, p2x - ox, p2y - oy);
}