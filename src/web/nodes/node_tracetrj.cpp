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

#include "node_tracetrj.h"
#include "md/particles.h"

INODE_DEF(__("Trace Trajectory"), TraceTrj, GEN)

Node_TraceTrj::Node_TraceTrj() : INODE_INIT{
	INODE_TITLE(NODE_COL_MOD)

	AddInput();
	scr.AddInput("values", "list(2d)");
}

void Node_TraceTrj::Execute() {
	has = false;
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].getconv();
	auto& sz = *cv.dimVals[0];
	if ((sz != Particles::anim.frameCount) || (*cv.dimVals[1] != 3)) return;
	pathSz = sz;
	path.resize(sz);
	memcpy(&path[0][0], *(float**)cv.value, sz * sizeof(Vec3));
	has = true;
}

void Node_TraceTrj::DrawScene() {
	if (!has || (pathSz != Particles::anim.frameCount) || pathSz == 1) return;

	auto fc = traceAll? Particles::anim.currentFrame + 1 : Particles::anim.frameCount;
	
	Engine::DrawLinesW(&path[0], fc, black(), 4);
}

void Node_TraceTrj::LoadOut(const std::string& path) {
    Execute();
}