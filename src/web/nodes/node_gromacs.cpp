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

#include "../anweb.h"
#include "node_gromacs.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#endif

Node_Gromacs::Node_Gromacs() : Node_Inputs() {
	script->name = ".ingro";
	title = "Gromacs File";
	titleCol = NODE_COL_IO;

	outputR.resize(4);
	script->outvars.resize(4);
	script->outvars[2].first = "residue IDs";
	script->outvars[3].first = "particle IDs";
	script->outvars[2].second = script->outvars[3].second = "list(1)";

	conV.resize(4);
	auto& ress = conV[2];
	ress.type = AN_VARTYPE::LIST;
	ress.dimVals.resize(1);
	conV[3] = ress;
}

void Node_Gromacs::Draw() {
	
}

void Node_Gromacs::Execute() {
	
}