#include "node_recolor.h"
#include "md/Particles.h"

Node_Recolor::Node_Recolor() : AnNode(new DmScript(".Recol")) {
	title = "Recolor";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
    canTile = false;
	inputR.resize(1);
	script->invars.push_back(std::pair<string, string>("grad", "list(1f)"));
}

void Node_Recolor::Execute() {
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& sz = *cv.dimVals[0];
	if (sz != Particles::particleSz) return;

	float* src = *((float**)cv.value);
	for (auto a = 0; a < sz; a++) {
        Particles::particles_Col[a] = (byte)Clamp<float>(roundf(255 * src[a]), 0, 255);
    }
    Particles::palleteDirty = true;
}

void Node_Recolor::LoadOut(const string& path) {
    Execute();
}

Node_Recolor_All::Node_Recolor_All() {
	script->name = ".RecolA";
	script->invars[0].second = "list(2f)";
}

void Node_Recolor_All::Execute() {
	if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& st = *cv.dimVals[0];
	auto& sz = *cv.dimVals[1];
	if (st != Particles::anim.frameCount || sz != Particles::particleSz) return;
	
	data.resize(st * sz);

	float* src = *((float**)cv.value);

	for (int a = 0; a < st * sz; a++) {
		data[a] = (byte)Clamp<float>(roundf(255 * src[a]), 0, 255);
	}
	OnAnimFrame();
}

void Node_Recolor_All::OnAnimFrame() {
	if (!data.size()) return;
	Particles::particles_Col = &data[Particles::particleSz * Particles::anim.activeFrame];
	Particles::palleteDirty = true;
}