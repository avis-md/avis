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

#include "node_setbboxcenter.h"
#include "md/particles.h"
#include "web/anweb.h"

INODE_DEF(__("Set Bounding Box Center"), SetBBoxCenter, SET)

Node_SetBBoxCenter::Node_SetBBoxCenter() : INODE_INIT{
	INODE_TITLE(NODE_COL_IO)
	INODE_SINIT(
		scr->AddInput(_("center"), AN_VARTYPE::DOUBLE, 1);
	);
}

void Node_SetBBoxCenter::Execute() {
	auto& ir = inputR[0];
	if (!ir.first) return;
	if (ir.getdim(0) != 3) {
		std::cerr << "vector must be of length 3!" << std::endl;
		return;
	}
	if (!AnWeb::execFrame)
		Particles::Rebound(**(glm::dvec3**)ir.getval(ANVAR_ORDER::C));
	else
		Particles::ReboundF(**(glm::dvec3**)ir.getval(ANVAR_ORDER::C), AnWeb::execFrame - 1);
}