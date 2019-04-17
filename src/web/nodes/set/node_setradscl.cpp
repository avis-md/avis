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