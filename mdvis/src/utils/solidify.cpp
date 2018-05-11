#include "solidify.h"

pMesh Solidify::Do(Vec3* path, uint cnt, float rad, uint dim, Vec3* str) {
	std::vector<Vec3> verts(cnt * dim), norms(cnt * dim);
	std::vector<int> tris((cnt-1) * dim * 2 * 3);

	Vec3* dirs = new Vec3[cnt];
	dirs[0] = Normalize(path[1] - path[0]);
	for (uint i = 1; i < cnt - 1; i++) {
		dirs[i] = Normalize(path[i+1] - path[i-1]);
	}
	dirs[cnt-1] = Normalize(path[cnt-1] - path[cnt-2]);
	
	Vec3* lops = new Vec3[cnt];
	Vec3* bilops = new Vec3[cnt];
	lops[0] = glm::cross(Vec3(1, 0, 0), dirs[0]);
	if (!glm::length(lops[0])) lops[0] = glm::cross(Vec3(0, 1, 0), dirs[0]);
	lops[0] = Normalize(lops[0]);
	bilops[0] = glm::cross(lops[0], dirs[0]);
	for (uint i = 1; i < cnt; i++) {
		bilops[i] = Normalize(glm::cross(lops[i-1], dirs[i]));
		lops[i] = glm::cross(dirs[i], bilops[i]);
	}

	float* cs = new float[dim] {};
	float* ss = new float[dim] {};
	cs[0] = 1;
	for (uint i = 1; i < dim; i++) {
		cs[i] = cosf(2 * PI * i / dim);
		ss[i] = sinf(2 * PI * i / dim);
	}

	for (uint i = 0; i < cnt; i++) {
		for (uint j = 0; j < dim; j++) {
			norms[i * dim + j] = cs[j] * lops[i] + ss[j] * bilops[i];
			verts[i * dim + j] = path[i] + norms[i * dim + j] * rad;
		}
	}

	for (uint i = 0; i < cnt - 1; i++) {
		for (uint j = 0; j < dim; j++) {
			uint off = i * dim + j;
			tris[off * 6] = off;
			tris[off * 6 + 1] = off + dim;
			tris[off * 6 + 3] = off + dim;
			if (j < dim - 1) {
				tris[off * 6 + 2] = off + 1;
				tris[off * 6 + 4] = off + dim + 1;
				tris[off * 6 + 5] = off + 1;
			}
			else {
				tris[off * 6 + 2] = i * dim;
				tris[off * 6 + 4] = (i + 1) * dim;
				tris[off * 6 + 5] = i * dim;
			}
		}
	}

	return std::make_shared<Mesh>(verts, norms, tris);
}