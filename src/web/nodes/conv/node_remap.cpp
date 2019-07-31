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

#include "node_remap.h"
#include "md/particles.h"
#include "ui/ui_ext.h"

INODE_DEF(__("Remap"), Remap, CONV)

Node_Remap::Node_Remap() : INODE_INIT {
	INODE_TITLE(NODE_COL_NRM);

	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(1f)"));

	outputR.resize(1);
	script->outvars.resize(1, std::pair<std::string, std::string>("result", "list(1f)"));
	
	conV.resize(1);
	auto& cv = conV[0];
	cv.type = AN_VARTYPE::LIST;
	cv.dimVals.resize(1);
	cv.value = &val0;
}

void Node_Remap::Execute() {
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].getconv();
	CVar& cvo = conV[0];
	auto sz = cvo.dimVals[0] = cv.dimVals[0];
	vals.resize(*sz);
	val0 = &vals[0];
	float* ins = *((float**)cv.value);
	for (int a = 0; a < *sz; ++a) {
		vals[a] = graph.Eval(ins[a]);
	}
}

void Node_Remap::DrawMiddle(float& off) {
	
}

void Node_Remap::LoadOut(const std::string& path) {
    Execute();
}