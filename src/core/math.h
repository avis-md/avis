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

template <typename T>
T Repeat(T t, T a, T b) {
	const T len = b - a;
	if (len <= 0) return a;
	const float frc = float(t - a) / len;
	return a + (T)(len * (frc - std::floor(frc)));
}
template <typename T>
T Clamp(T t, T a, T b) {
	return std::min(b, std::max(t, a));
}

Vec2 Clamp(Vec2 t, Vec2 a, Vec2 b);

template <typename T>
T Lerp(T a, T b, float c) {
	if (c < 0) return a;
	else if (c > 1) return b;
	else return a*(1 - c) + b*c;
}
template <typename T>
float InverseLerp(T a, T b, T c) {
	return Clamp((float)((c - a) / (b - a)), 0.f, 1.f);
}