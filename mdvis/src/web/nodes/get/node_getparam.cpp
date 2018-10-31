#include "node_getparam.h"
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "web/anweb.h"

Node_GetParam::Node_GetParam() : AnNode(new DmScript(sig)), paramId(0), di(&paramId, &Particles::attrNms[0]) {
	title = "Get Attribute";
	titleCol = NODE_COL_IO;
	outputR.resize(1);
	script->outvars.resize(1, std::pair<std::string, std::string>("values", "list(1d)"));
	conV.resize(1);
	conVAll.resize(1);
	auto& cv = conV[0];
	cv.type = AN_VARTYPE::DOUBLE;
	cv.dimVals.resize(1);
	cv.value = &cv.data.val.arr.p;
}

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

void Node_GetParam::Execute() {
	if (!Particles::attrs.size())
		RETERR("No attribute available!");
	auto& cv = conV[0];
	cv.dimVals[0] = (int*)&Particles::particleSz;
	cv.data.val.arr.p = Particles::attrs[paramId]->Get(AnWeb::execFrame).data();
}

void Node_GetParam::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI::Quad(pos.x, off, width, 18, bgCol);
	UI2::Dropdown(pos.x + 2, off, width - 4, "Attribute", di);
	off += 18;
}