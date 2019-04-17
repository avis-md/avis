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