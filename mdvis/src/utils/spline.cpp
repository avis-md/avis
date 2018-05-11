#include "spline.h"

#define FLerp(p1, p2, f) (p1 * (1-f) + p2 * f)

Vec3 Spline::Bezier(const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& p4, float t) {
	Vec3 pa1 = FLerp(p1, p2, t);
	Vec3 pa2 = FLerp(p2, p3, t);
	Vec3 pa3 = FLerp(p3, p4, t);
	Vec3 pb1 = FLerp(pa1, pa2, t);
	Vec3 pb2 = FLerp(pa2, pa3, t);
	return FLerp(pb1, pb2, t);
}

void Spline::ToSpline(Vec3* pts, uint cnt, uint dim, Vec3* res) {
	Vec3* cps = new Vec3[cnt * 3 - 2];
	cps[0] = pts[0];
	cps[(cnt-1)*3] = pts[cnt-1];
	for (uint a = 0; a < cnt - 1; a++) {
		cps[a * 3 + 1] = FLerp(pts[a], pts[a + 1], 0.33333f);
		cps[a * 3 + 2] = FLerp(pts[a], pts[a + 1], 0.66667f);
	}
	for (uint a = 1; a < cnt - 1; a++) {
		cps[a * 3] = FLerp(cps[a * 3 - 1], cps[a * 3 + 1], 0.5f);
	}

	//memcpy(res, cps, (cnt * 3 - 2) * sizeof(Vec3));
	//return;
	
	//res[0] = pts[0];
	res[(cnt-1) * dim] = pts[cnt-1];
	for (uint a = 0; a < cnt - 1; a++) {
		res[a * dim] = cps[a * 3];
		for (byte b = 1; b < dim; b++) {
			res[a * dim + b] = Bezier(cps[a * 3], cps[a * 3 + 1], cps[a * 3 + 2], cps[a * 3 + 3], (float)(b) / dim);
		}
	}
}