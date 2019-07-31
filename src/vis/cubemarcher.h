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

class CubeMarcher {
public:
	CubeMarcher(Texture3D* tex);

	static void Init();

	void Draw(const Mat4x4& _mv, const Mat4x4& _p);

	Vec3 pos, scl;
	Quat rot;

	Texture3D* tex;

private:
	static int32_t _lookupTable[256*16];
	static GLuint lookupBuf, lookupTex;

	static GLuint prog;
};