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

#include "node_setradscl.h"
#include "md/particles.h"

INODE_DEF(__("Set Radius Scale"), SetRadScl, SET)

Node_SetRadScl::Node_SetRadScl() : INODE_INITF(AN_FLAG_RUNONSEEK) {
	INODE_TITLE(NODE_COL_IO);
	INODE_SINIT(
		scr->AddInput(_("scale"), AN_VARTYPE::DOUBLE, 1);
	);
}

void Node_SetRadScl::Execute() {
    if (!inputR[0].first) return;
	auto& ir = inputR[0];
	if (ir.getdim(0) != Particles::particleSz) return;

	double* vals = *(double**)ir.getval(ANVAR_ORDER::C);

#pragma omp parallel for
    for (int a = 0; a < Particles::particleSz; ++a) {
        Particles::radiiscl[a] = std::max((float)vals[a], 0.0001f);
    }
    Particles::visDirty = true;
	canReset = true;
}

void Node_SetRadScl::DrawFooter(float& off) {
	if (canReset) {
		if (Engine::Button(pos.x + 2, off, width - 4, 16, white(1, 0.4f), "Reset", 12, white(), true) == MOUSE_RELEASE) {
#pragma omp parallel for
			for (int a = 0; a < Particles::particleSz; ++a) {
				Particles::radiiscl[a] = 1;
			}
			Particles::visDirty = true;
			canReset = false;
		}
		off += 17;
	}
}

void Node_SetRadScl::LoadOut(const std::string& path) {
    Execute();
}