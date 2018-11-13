#include "node_setattribute.h"
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "web/anweb.h"

Node_SetAttribute::Node_SetAttribute() : AnNode(new DmScript(sig)), attrId(0), _attrId(-1), attrSz(-1), di(&attrId, nullptr), timed(true) {
	title = "Set Attribute";
	titleCol = NODE_COL_IO;
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(1*)"));
}

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

void Node_SetAttribute::Execute() {
    if (!inputR[0].first) return;
	if (!Particles::attrs.size()) {
		RETERR("No attribute available!");
	}
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto sz = *cv.dimVals[0];
	if (sz != Particles::particleSz)
		RETERR("Attribute must be for each atom!");
	auto src = *((void**)cv.value);
	auto prm = Particles::attrs[attrId];
	prm->timed = (AnWeb::execFrame > 0 && timed);
	auto& tar = prm->Get(prm->timed? AnWeb::execFrame-1 : 0);
	tar.resize(sz);
	switch (cv.typeName[6]) {
	case 's':
		for (int i = 0; i < sz; ++i) {
			tar[i] = ((short*)src)[i];
		}
		break;
	case 'i':
		for (int i = 0; i < sz; ++i) {
			tar[i] = ((int*)src)[i];
		}
		break;
	case 'd':
		memcpy(tar.data(), src, sz * sizeof(double));
		break;
	default:
		RETERR("Unexpected data type " + cv.typeName + "!");
	}
	prm->dirty = true;
}

void Node_SetAttribute::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI::Quad(pos.x, off, width, 35, bgCol);
	if (attrSz != Particles::attrs.size()) {
		attrSz = Particles::attrs.size();
		attrs.clear();
		for (uint a = 0; a < attrSz; ++a) {
			if (!Particles::attrs[a]->readonly) {
				attrs.push_back(Particles::attrNms[a]);
			}
		}
		if (attrId >= attrs.size()) {
			attrId = (uint)std::max((int)attrs.size()-1, 0);
		}
		attrs.push_back("<create new>");
		attrs.push_back("");
		di.list = &attrs[0];
	}
	UI2::Dropdown(pos.x + 2, off, width - 4, "Attribute", di);
	off += 17;
	if (attrId != _attrId) {
		_attrId = attrId;
		if (attrId == attrs.size()-2) {
			Particles::AddParam();
		}
	}
	UI2::Toggle(pos.x + 2, off, width - 4, "Animated", timed);
	off += 18;
}

void Node_SetAttribute::LoadOut(const std::string& path) {
	Execute();
}