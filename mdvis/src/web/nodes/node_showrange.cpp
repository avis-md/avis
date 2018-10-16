#include "node_showrange.h"
#include "md/particles.h"
#include "vis/pargraphics.h"
#include "ui/ui_ext.h"

Node_ShowRange::Node_ShowRange() : AnNode(new DmScript(sig)) {
	title = "Show Range";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
    canTile = false;
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(1d)"));
}

void Node_ShowRange::Execute() {
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& sz = *cv.dimVals[0];
	if (sz != Particles::particleSz) return;

    double* vals = *((double**)cv.value);

#pragma omp parallel for
    for (int a = 0; a < Particles::particleSz; a++) {
        auto& v = vals[a];
        if (v > rMin && v < rMax)
            Particles::visii[a] = !invert;
        else
            Particles::visii[a] = invert;
    }
    Particles::visDirty = true;
}

void Node_ShowRange::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI::Quad(pos.x, off, width, 17*3+2, bgCol);
	UI2::Toggle(pos.x + 2, off, width - 4, "invert", invert);
    title = invert? "Hide Range" : "Show Range";
    off += 18;
    auto s = std::to_string(rMin);
    s = UI2::EditText(pos.x + 2, off, (uint)width - 4, "min", s);
    rMin = TryParse(s, 0.f);
    s = std::to_string(rMax);
    s = UI2::EditText(pos.x + 2, off + 17, (uint)width - 4, "max", s);
    rMax = TryParse(s, 0.f);
	off += 35;
}

void Node_ShowRange::LoadOut(const std::string& path) {
    Execute();
}