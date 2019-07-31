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
#include "../annode.h"

class Node_Volume : public AnNode {
public:
	static const std::string sig;
	Node_Volume();

	static void Init();
	Vec2 DrawConn() override;
	void Draw() override;
	float DrawSide() override;
	void DrawScene() override;
	void Execute() override;
	
protected:
	static const Vec3 cubeVerts[8];
	static const uint cubeIndices[36];
	float ox, oy, oz, sx, sy, sz;
	int nx, ny, nz;
	Vec3 cutC, cutD;
	GLuint tex;
	static GLuint shad;
	static GLint shadLocs[10];
	static GLuint vao, vbo, veo;
};