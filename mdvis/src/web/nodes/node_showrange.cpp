#include "node_showrange.h"
#include "md/Particles.h"
#include "vis/pargraphics.h"
#include "ui/ui_ext.h"

Node_ShowRange::Node_ShowRange() : AnNode(new DmScript(sig)) {
	title = "Show Range";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
    canTile = false;
	inputR.resize(1);
	script->invars.push_back(std::pair<string, string>("values", "list(1f)"));
}

void Node_ShowRange::Execute() {
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& sz = *cv.dimVals[0];
	if (sz != Particles::particleSz) return;

    float* vals = *((float**)cv.value);

    //
    for (auto& r : Particles::residueLists) {
        r.visible = r.visibleAll = true;
        int a = -1;
        for (auto& rr : r.residues) {
            auto& v = vals[rr.offset];
            if (v > rMin && v < rMax) {
                rr.visible = !invert;
            }
            else rr.visible = invert;
            if (a == -1) {
                r.visible = rr.visible;
                a = rr.visible? 1 : 0;
            }
            else if (!a == rr.visible) r.visibleAll = false;
        }
    }
    ParGraphics::UpdateDrawLists();
}

void Node_ShowRange::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	Engine::DrawQuad(pos.x, off, width, 17*3+2, white(0.7f, 0.25f));
	UI2::Toggle(pos.x + 2, off, width - 4, "invert", invert);
    title = invert? "Hide Range" : "Show Range";
    off += 18;
    auto s = std::to_string(rMin);
    s = UI2::EditText(pos.x + 2, off, width - 4, "min", s);
    rMin = TryParse(s, 0.0f);
    s = std::to_string(rMax);
    s = UI2::EditText(pos.x + 2, off + 17, width - 4, "max", s);
    rMax = TryParse(s, 0.0f);
	off += 35;
    hdSz = 17 * 3 + 2;
}

void Node_ShowRange::LoadOut(const string& path) {
    Execute();
}