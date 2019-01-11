#include "node_dowhile.h"
#include "web/anweb.h"
#include "ui/ui_ext.h"

INODE_DEF(__("Loop While"), DoWhile, MISC)

Node_DoWhile::Node_DoWhile() : INODE_INITF(AN_FLAG_NOSAVECONV),
		start(0), type(COMP::GT), _nodeCnt(~0U) {
	INODE_TITLE(NODE_COL_CTRL);
	
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

	UI2::Dropdown(pos.x + 5, off, width - 10, "From", tDi); off += 17;
	UI2::Dropdown(pos.x + 5, off, width - 10, "Comparator", cDi); off += 17;
}

void Node_DoWhile::Execute() {
	double vl = getval_d(0);
	double cmp = getval_d(1);
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