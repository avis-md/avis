#include "node_setparam.h"
#include "md/Particles.h"
#include "ui/ui_ext.h"

Node_SetParam::Node_SetParam() : AnNode(new DmScript(sig)), paramId(0), di(&paramId, Particles::particles_ParamNms) {
	title = "Set Parameter";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
    canTile = false;
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(1f)"));
}

void Node_SetParam::Execute() {
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& sz = *cv.dimVals[0];
	if (sz != Particles::particleSz) return;

    Particles::particles_Params[paramId]->data = *((float**)cv.value);
    Particles::particles_Params[paramId]->dirty = true;
}

void Node_SetParam::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI::Quad(pos.x, off, width, 18, bgCol);
	UI2::Dropdown(pos.x + 2, off, width - 4, "Parameter", di);
	off += 18;
}

void Node_SetParam::LoadOut(const std::string& path) {
    Execute();
}