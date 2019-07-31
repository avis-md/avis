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

#pragma once
#include "Engine.h"

class UI3 {
public:
	static void Line(Vec3 p1, Vec3 p2, float w, Vec4 col);
	static void Path(Vec3* path, uint cnt, float w, Vec4 col);
	static void Cube(Vec3 pos, float dx, float dy, float dz, Vec4 col);
	static void Cube(float x1, float x2, float y1, float y2, float z1, float z2, Vec4 col);
	static void Init();

private:
	static GLuint cubeProgIds;
};