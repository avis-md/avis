#include "arrow.h"

Arrow::Arrow(float w1, float h1, float w2, float h2, int res) {
	verts.reserve(6 * res);
	norms.reserve(6 * res);
	tris.reserve(18 * res);

	for (int a = 0; a < res; a++) {
		float w = a*2*PI/res;
		float cs = cosf(w);
		float sn = sinf(w);
		Vec2 wv = Vec2(cs, sn);
		float w12 = w1 + w2;
		
		verts.push_back(Vec3(wv.x*w1, 0, wv.y*w1));
		verts.push_back(Vec3(wv.x*w1, h1, wv.y*w1));
		verts.push_back(verts.back());
		verts.push_back(Vec3(wv.x*(w12), h1, wv.y*(w12)));
		verts.push_back(verts.back());
		verts.push_back(Vec3(0, h1 + h2, 0));

		norms.push_back(Vec3(wv.x, 0, wv.y));
		norms.push_back(norms.back());
		norms.push_back(Vec3(0, -1, 0));
		norms.push_back(norms.back());
		norms.push_back(glm::normalize(Vec3(wv.x*h2, w12, wv.y*h2)));
		norms.push_back(norms.back());

		int v0 = a*6;
		int v1 = ((a+1)%res)*6;
		tris.insert(tris.end(), {
			v0, v0+1, v1,		v0+1, v1+1, v1,
			v0+2, v0+3, v1+2,	v0+3, v1+3, v1+2,
			v0+4, v0+5, v1+4,	v0+5, v1+1, v1+4
		});
	}
}