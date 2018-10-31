#include "node_setparam.h"
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "web/anweb.h"

Node_SetParam::Node_SetParam() : AnNode(new DmScript(sig)), paramId(0), di(&paramId, &Particles::attrNms[0]) {
	title = "Set Attribute";
	titleCol = NODE_COL_IO;
    canTile = false;
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(1*)"));
}

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

void Node_SetParam::Execute() {
    if (!inputR[0].first) return;
	if (!Particles::attrs.size()) {
		RETERR("No attribute available!");
	}
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto sz = *cv.dimVals[0];
	if (sz != Particles::particleSz)
		RETERR("Attribute must be for each atom!");
	auto src = *((void**)cv.value);
	auto prm = Particles::attrs[paramId];
	prm->timed = (AnWeb::execFrame > 0);
	auto& tar = prm->Get(prm->timed? AnWeb::execFrame-1 : 0);
	tar.resize(sz);
	switch (cv.typeName[6]) {
	case 's':
		for (int i = 0; i < sz; ++i)  {
			tar[i] = ((short*)src)[i];
		}
		break;	
	case 'i':
		for (int i = 0; i < sz; ++i)  {
			tar[i] = ((int*)src)[i];
		}
		break;
	case 'd':
		for (int i = 0; i < sz; ++i)  {
			tar[i] = ((double*)src)[i];
		}
		break;
	default:
		RETERR("Unexpected data type " + cv.typeName + "!");
	}
	prm->dirty = true;
}

void Node_SetParam::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI::Quad(pos.x, off, width, 18, bgCol);
	UI2::Dropdown(pos.x + 2, off, width - 4, "Attribute", di);
	off += 18;
}

void Node_SetParam::LoadOut(const std::string& path) {
	Execute();
}