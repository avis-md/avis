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