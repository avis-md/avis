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

#include "node_inputs_v.h"
#include "web/anweb.h"
#ifndef IS_ANSERVER
#include "md/volumetric.h"
#include "ui/ui_ext.h"
#include "vis/pargraphics.h"
#endif

INODE_DEF(__("Volumetric Data"), InputsV, GET)

Node_InputsV::Node_InputsV() : INODE_INITF(AN_FLAG_NOSAVECONV) {
	INODE_TITLE(NODE_COL_IO);
	INODE_SINIT(
		scr->desc = R"(Volumetric data
        )";
		scr->descLines = 1;
		
		scr->AddOutput(_("values"), AN_VARTYPE::DOUBLE, 3);
	);

	IAddConV(0, { &szs[0], &szs[1], &szs[2] }, { });
}

void Node_InputsV::Execute() {
	szs[0] = Volumetric::currentFrame->nx;
	szs[1] = Volumetric::currentFrame->ny;
	szs[2] = Volumetric::currentFrame->nz;
	auto& ov = ((DmScript_I*)script.get())->outputVs;
	ov[0].val.p = Volumetric::currentFrame->data.data();
}