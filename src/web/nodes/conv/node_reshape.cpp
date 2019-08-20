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

#include "node_reshape.h"
#include "ui/ui_ext.h"

#define MAX_RESHAPE_DIM 10

INODE_DEF(__("Reshape"), Reshape, CONV)

Node_Reshape::Node_Reshape() : INODE_INIT, dim(2), lastsz(0) {
	INODE_TITLE(NODE_COL_NRM);
	INODE_SINIT(

		scr->AddInput(_("list"), AN_VARTYPE::INT, 2);

		for (int a = 1; a <= MAX_RESHAPE_DIM; a++) {
			scr->AddInput(_("dim") + " " + std::to_string(a), AN_VARTYPE::INT);
		}

		scr->AddOutput(_("result"), AN_VARTYPE::INT, 2);
	);
	for (int a = dim + 1; a <= MAX_RESHAPE_DIM; a++) {
		useInputs[a] = false;
	}

	IAddConV(0, { &getval_i(1), &lastsz }, {});
}

void Node_Reshape::DrawHeader(float& off) {
	const auto v = TryParse(UI2::EditText(pos.x + 2, off, width - 4, "dim", std::to_string(dim)), 2);
	if (v != dim) {
		std::vector<int*> dvs;
		for (int a = 1; a <= dim; a++) {
			useInputs[a] = true;
			dvs.push_back(&getval_i(a));
		}
		for (int a = dim + 1; a <= MAX_RESHAPE_DIM; a++) {
			useInputs[a] = false;
		}
		dvs.push_back(&lastsz);
		ISetConDim(0, dvs, {});
	}
}

void Node_Reshape::Execute() {
    if (!inputR[0].first) return;
	
	int d = 1;
	for ()

	for (int a = 1; )

	auto& ov = ((DmScript_I*)script.get())->outputVs;
	ov[0].pval = inputR[0].getval(ANVAR_ORDER::C);
}