#include "node_tracetrj.h"
#include "md/particles.h"

INODE_DEF(__("Trace Trajectory"), TraceTrj, GEN)

Node_TraceTrj::Node_TraceTrj() : INODE_INIT{
	INODE_TITLE(NODE_COL_MOD)

	AddInput();
	scr.AddInput("values", "list(2d)");
}

void Node_TraceTrj::Execute() {
	has = false;
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].getconv();
	auto& sz = *cv.dimVals[0];
	if ((sz != Particles::anim.frameCount) || (*cv.dimVals[1] != 3)) return;
	pathSz = sz;
	path.resize(sz);
	memcpy(&path[0][0], *(float**)cv.value, sz * sizeof(Vec3));
	has = true;
}

void Node_TraceTrj::DrawScene() {
	if (!has || (pathSz != Particles::anim.frameCount) || pathSz == 1) return;

	auto fc = traceAll? Particles::anim.currentFrame + 1 : Particles::anim.frameCount;
	
	Engine::DrawLinesW(&path[0], fc, black(), 4);
}

void Node_TraceTrj::LoadOut(const std::string& path) {
    Execute();
}