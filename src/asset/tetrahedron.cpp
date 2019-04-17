#include "tetrahedron.h"

Tetrahedron::Tetrahedron() {
	const float c = 1 / std::sqrt(2.f);
	vertices = { Vec3(1, 0, -c),
		Vec3(-1, 0, -c),
		Vec3(0, 1, c),
		Vec3(0, -1, c) };

	triangles = { 0, 2, 1, 
		1, 2, 3, 
		2, 0, 3, 
		3, 0, 1 };

	vertCount = 4;
	triCount = 4;
}

#define mrg(a, b) ((int64_t)a << 32) + (int64_t)b

void Tetrahedron::Subdivide() {
	auto tsz = triCount * 3;
	vertices.resize(vertCount + tsz/2);
	auto triso = triangles;
	triangles.resize(tsz * 4);

	std::unordered_map<int64_t, int> map;
	for (size_t a = 0; a < tsz/3; ++a) {
		int* vs = &triso[a*3];
		int vn[3];
		for (int b = 0; b < 3; ++b) {
			auto vs0 = vs[b];
			auto vs1 = vs[(b==2)? 0 : b+1];
			int in = map[mrg(vs0, vs1)];
			if (!in) {
				vertices[vertCount] = Lerp(vertices[vs0], vertices[vs1], 0.5f);
				vn[b] = vertCount;
				map[mrg(vs0, vs1)] = map[mrg(vs1, vs0)] = ++vertCount;
			}
			else {
				vn[b] = in - 1;
			}
		}

		int tt[] = { vs[0], vn[0], vn[2], vs[1], vn[1], vn[0], 
			vs[2], vn[2], vn[1], vn[0], vn[1], vn[2] };

		memcpy(&triangles[a*12], tt, 12*sizeof(int));
	}
	triCount *= 4;
	assert(vertCount == vertices.size());
	assert(triCount == triangles.size() / 3);
}

void Tetrahedron::ToSphere(float rad) {
	auto vsz = vertices.size();
	normals.resize(vsz);
	for (size_t a = 0; a < vsz; ++a) {
		normals[a] = glm::normalize(vertices[a]);
		vertices[a] = normals[a] * rad;
	}
}