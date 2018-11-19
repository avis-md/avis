#include "node_setradscl.h"
#include "md/particles.h"

Node_SetRadScl::Node_SetRadScl() : AnNode(new DmScript(sig), AN_FLAG_RUNONSEEK) {
	title = "Set Radius Scale";
	titleCol = NODE_COL_IO;
    canTile = false;
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(1d)"));
}

void Node_SetRadScl::Execute() {
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& sz = *cv.dimVals[0];
	if (sz != Particles::particleSz) return;

    double* vals = *((double**)cv.value);

#pragma omp parallel for
    for (int a = 0; a < Particles::particleSz; ++a) {
        Particles::radiiscl[a] = std::max((float)vals[a], 0.0001f);
    }
    Particles::visDirty = true;
}

void Node_SetRadScl::LoadOut(const std::string& path) {
    Execute();
}