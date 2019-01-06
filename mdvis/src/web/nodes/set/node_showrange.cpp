#include "node_showrange.h"
#include "md/particles.h"
#include "vis/pargraphics.h"
#include "ui/ui_ext.h"

INODE_DEF(__("Show Range"), ShowRange, SET)

Node_ShowRange::Node_ShowRange() : INODE_INIT, invert(false), rMin(0), rMax(1) {
	INODE_TITLE(NODE_COL_MOD);
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
    for (int a = 0; a < Particles::particleSz; ++a) {
        auto& v = vals[a];
        if (v >= rMin && v <= rMax)
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

void Node_ShowRange::Save(XmlNode* n) {
	n->addchild("invert", invert? "1" : "0");
    n->addchild("min", std::to_string(rMin));
    n->addchild("max", std::to_string(rMax));
}

void Node_ShowRange::Load(XmlNode* n2) {
	for (auto& n : n2->children) {
		if (n.name == "invert") invert = (n.value == "1");
		else if (n.name == "min") rMin = TryParse(n.value, 0.0f);
		else if (n.name == "max") rMax = TryParse(n.value, 1.0f);
	}
}

void Node_ShowRange::LoadOut(const std::string& path) {
    Execute();
}