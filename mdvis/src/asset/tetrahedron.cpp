#include "tetrahedron.h"

Mesh Tetrahedron::New(float rad, int quality) {
	int nvert = 4 + 6 * quality + 2 * quality * (quality + 1);
	int ntri = 0;
	for (int a = 0; a <= quality; a++) {
		ntri += 2 * a + 1;
	}
	ntri *= 4;
	std::vector<Vec3> verts(nvert);
	std::vector<int> tris(ntri * 3);

	const float c = 1 / std::sqrtf(2);
	verts[0] = Vec3(1, 0, -c);
	verts[1] = Vec3(-1, 0, -c);
	verts[2] = Vec3(0, 1, c);
	verts[3] = Vec3(0, -1, c);

	std::array<int, 3> bt[] = {
		{0, 2, 1}, {1, 2, 3},
		{2, 0, 3}, {3, 0, 1}
	};

	std::array<int, 2> be[] = {
		{0, 1}, {0, 2},
		{1, 3}, {1, 2},
		{2, 3}, {2, 0},
		{3, 1}, {3, 0}
	};

	std::unordered_map<Int2, Int2> voff;

	int off = 4;
	for (int a = 0; a < 8; a++) {
		int vi1 = be[a][0];
		int vi2 = be[a][1];
		Vec3 v1 = verts[vi1];
		Vec3 v2 = verts[vi2];
		voff.emplace(Int2(vi1, vi2), Int2(off, 1));
		for (int b = 0; b < quality; b++) {
			verts[off++] = Lerp(v1, v2, (b+1.0f)/(quality+1));
		}
		voff.emplace(Int2(vi2, vi1), Int2(off - 1, -1));
	}
}