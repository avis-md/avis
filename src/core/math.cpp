#include "math.h"

Vec2 Clamp(Vec2 t, Vec2 a, Vec2 b) {
	return Vec2(Clamp(t.x, a.x, b.x), Clamp(t.y, a.y, b.y));
}