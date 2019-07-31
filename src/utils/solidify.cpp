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

#include "solidify.h"

void Solidify::GetTangents(Vec3* path, uint cnt, Vec3* tans) {
	Vec3* dirs = new Vec3[cnt];
	dirs[0] = glm::normalize(path[1] - path[0]);
	for (uint i = 1; i < cnt - 1; ++i) {
		dirs[i] = glm::normalize(path[i+1] - path[i-1]);
	}
	dirs[cnt-1] = glm::normalize(path[cnt-1] - path[cnt-2]);

	tans[0] = glm::cross(Vec3(1, 0, 0), dirs[0]);
	if (!glm::length(tans[0])) tans[0] = glm::cross(Vec3(0, 1, 0), dirs[0]);
	tans[0] = glm::normalize(tans[0]);
	for (uint i = 1; i < cnt; ++i) {
		auto tmp = glm::normalize(glm::cross(tans[i-1], dirs[i]));
		tans[i] = glm::cross(dirs[i], tmp);
	}
	delete[](dirs);
}

pMesh Solidify::Do(Vec3* path, uint cnt, float rad, uint dim, Vec3* str) {
	std::vector<Vec3> verts(cnt * dim), norms(cnt * dim);
	std::vector<int> tris((cnt-1) * dim * 2 * 3);

	Vec3* dirs = new Vec3[cnt];
	dirs[0] = glm::normalize(path[1] - path[0]);
	for (uint i = 1; i < cnt - 1; ++i) {
		dirs[i] = glm::normalize(path[i+1] - path[i-1]);
	}
	dirs[cnt-1] = glm::normalize(path[cnt-1] - path[cnt-2]);
	
	Vec3* lops = new Vec3[cnt];
	Vec3* bilops = new Vec3[cnt];
	lops[0] = glm::cross(Vec3(1, 0, 0), dirs[0]);
	if (!glm::length(lops[0])) lops[0] = glm::cross(Vec3(0, 1, 0), dirs[0]);
	lops[0] = glm::normalize(lops[0]);
	bilops[0] = glm::cross(lops[0], dirs[0]);
	for (uint i = 1; i < cnt; ++i) {
		bilops[i] = glm::normalize(glm::cross(lops[i-1], dirs[i]));
		lops[i] = glm::cross(dirs[i], bilops[i]);
	}

	float* cs = new float[dim];
	float* ss = new float[dim];
	cs[0] = 1;
	ss[0] = 0;
	for (uint i = 1; i < dim; ++i) {
		cs[i] = cosf(2 * PI * i / dim);
		ss[i] = sinf(2 * PI * i / dim);
	}

	for (uint i = 0; i < cnt; ++i) {
		for (uint j = 0; j < dim; ++j) {
			norms[i * dim + j] = cs[j] * lops[i] + ss[j] * bilops[i];
			verts[i * dim + j] = path[i] + norms[i * dim + j] * rad;
		}
	}

	for (uint i = 0; i < cnt - 1; ++i) {
		for (uint j = 0; j < dim; ++j) {
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