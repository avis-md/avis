#include "node_dowhile.h"
#include "web/anweb.h"
#include "ui/ui_ext.h"

Node_DoWhile::Node_DoWhile() : AnNode(new DmScript(sig), AN_FLAG_NOSAVECONV),
		start(0), type(COMP::GT), _nodeCnt(~0U) {
	title = "Loop While";
	titleCol = NODE_COL_SPC;
	AddInput();
	script->AddInput("value", "double");
	AddInput();
	script->AddInput("target", "double");

	static std::string ss[] = { "Greater Than", "Less Than", "Not Equal To", "Equal To", "" };
	cDi.target = (uint*)&type;
	cDi.list = &ss[0];
	tDi.target = &start;
}


void Node_DoWhile::DrawHeader(float& off) {
	AnNode::DrawHeader(off);

	if (AnWeb::nodes.size() != _nodeCnt) {
		_nodeCnt = AnWeb::nodes.size();
		nodeNms.clear();
		uint loc = (uint)(std::find(AnWeb::nodes.begin(), AnWeb::nodes.end(), this) - AnWeb::nodes.begin());
		nodeNms.resize(loc + 1);
		for (uint a = 0; a < loc; ++a) {
			nodeNms[a] = std::to_string(a+1) + ". " + AnWeb::nodes[a]->title;
		}
		tDi.list = nodeNms.data();
		start = std::min(start, loc - 1U);
	}

	UI::Quad(pos.x, off, width, 34, bgCol);
	UI2::Dropdown(pos.x + 5, off, width - 10, "From", tDi); off += 17;
	UI2::Dropdown(pos.x + 5, off, width - 10, "Comparator", cDi); off += 17;
}

void Node_DoWhile::Execute() {
	double vl = inputR[0].first? *(double*)inputR[0].getval() : inputVDef[0].d;
	double cmp = inputR[1].first? *(double*)inputR[1].getval() : inputVDef[1].d;
	bool loop = false;
	switch (type) {
	case COMP::GT:
		loop = vl > cmp;
		break;
	case COMP::LT:
		loop = vl < cmp;
		break;
	case COMP::NEQ:
		loop = vl != cmp;
		break;
	case COMP::EQ:
		loop = vl == cmp;
		break;
	}

	if (loop) {
		AnWeb::nextNode = start;
	}
}