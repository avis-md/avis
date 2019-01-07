#include "node_setradscl.h"
#include "md/particles.h"

INODE_DEF(__("Set Radius Scale"), SetRadScl, SET)

Node_SetRadScl::Node_SetRadScl() : INODE_INITF(AN_FLAG_RUNONSEEK) {
	INODE_TITLE(NODE_COL_IO);

	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(1d)"));
}

void Node_SetRadScl::Execute() {
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].getconv();
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