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