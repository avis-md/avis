#include "tube.h"

Tube::Tube(int div, float r, float h) {
	vertCount = div * 2;
	triCount = div * 2;
	vertices.reserve(vertCount);
	normals.reserve(vertCount);
	triangles.reserve(triCount * 3);

	const float mul = 2 * PI / div;
	for (int a = 0; a < div; a++) {
		float x = std::cos(a * mul);
		float y = std::sin(a * mul);
		vertices.push_back(Vec3(x * r, y * r, 0));
		vertices.push_back(Vec3(x * r, y * r, h));
		normals.push_back(Vec3(x, y, 0));
		normals.push_back(Vec3(x, y, 0));

		int b = (a == div - 1) ? 0 : a + 1;
		triangles.insert(triangles.end(), { a * 2,  a * 2 + 1, b * 2, a * 2 + 1, b * 2 + 1, b * 2});
	}
}