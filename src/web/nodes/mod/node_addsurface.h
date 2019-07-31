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
#include "web/annode_internal.h"
#include "ui/popups.h"

class Node_AddSurface : public AnNode {
public:
	INODE_DEF_H
	Node_AddSurface();
	~Node_AddSurface();

	void Update() override;
	void DrawHeader(float& off) override;
	void DrawScene(const RENDER_PASS pass) override;

	void RayTraceMesh(_Mesh& mesh) override;
	
	void Execute() override;
protected:
	enum class Mode : uint {
		Draw,
		Gen
	} mode;
	Popups::DropdownItem mode_di;

	static Shader marchProg, drawProg;

	std::vector<float> data;
	int shape[3];
	float cutoff;
	bool invert = false;

	std::vector<Vec3> resultPos, resultNrm;
	std::vector<glm::dvec3> resultPosd;

	static size_t bufSz;
	static int maxBufSz;
	static uint genSz;
	static GLuint inBuf, inBufT, query;
	GLuint vao, outPos, outNrm, tmpPos, tmpNrm;

	static std::mutex lock;

	static void Init();
	void InitBuffers();

	void ResizeInBuf(int), ResizeOutBuf(int), ResizeTmpBuf(int);
	void SetInBuf(void*, int);
	int ExecMC(glm::ivec3 offset, glm::ivec3 size, glm::ivec3 rsize);

	static const int triTable[256*15];
	static GLuint triBuf, triBufT;
};