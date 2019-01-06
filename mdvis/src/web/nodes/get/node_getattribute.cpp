#include "node_getattribute.h"
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "web/anweb.h"

INODE_DEF(__("Get Attribute"), GetAttribute, GET)

Node_GetAttribute::Node_GetAttribute() : 
		INODE_INITF(AN_FLAG_NOSAVECONV | AN_FLAG_RUNONSEEK),
		attrId(0), di(&attrId, &Particles::attrNms[0]) {
	INODE_TITLE(NODE_COL_IO);

	AddOutput(CVar("values", 'd', 1, { (int*)&Particles::particleSz }));
	scr.AddOutput(conV[0]);
	auto& cv = conV[0];
	cv.value = &cv.data.val.arr.p;
}

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

void Node_GetAttribute::Execute() {
	if (!Particles::attrs.size())
		RETERR("No attribute available!");
	auto& cv = conV[0];
	cv.data.val.arr.p = Particles::attrs[attrId]->Get(
		(!AnWeb::execFrame)? Particles::anim.currentFrame : AnWeb::execFrame-1).data();
}

void Node_GetAttribute::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI::Quad(pos.x, off, width, 18, bgCol);
	UI2::Dropdown(pos.x + 2, off, width - 4, "Attribute", di);
	off += 18;
}

void Node_GetAttribute::Save(XmlNode* n) {
	n->addchild("id", std::to_string(attrId));
}

void Node_GetAttribute::Load(XmlNode* n2) {
	for (auto& n : n2->children) {
		if (n.name == "id") {
			attrId = TryParse(n.value, 0U);
			attrId = std::min(attrId, (uint)((int)Particles::attrs.size()-1));
		}
	}
}