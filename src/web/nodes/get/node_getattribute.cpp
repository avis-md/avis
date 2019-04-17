#include "node_getattribute.h"
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "web/anweb.h"

INODE_DEF(__("Get Attribute"), GetAttribute, GET)

Node_GetAttribute::Node_GetAttribute() : 
		INODE_INITF(AN_FLAG_NOSAVECONV | AN_FLAG_RUNONSEEK),
		attrId(0), di(&attrId, &Particles::attrNms[0]) {
	INODE_TITLE(NODE_COL_IO);
	INODE_SINIT(
		scr->AddOutput(_("values"), AN_VARTYPE::DOUBLE, 1);
	);

	IAddConV(0, { (int*)&Particles::particleSz }, {});

}

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

void Node_GetAttribute::Execute() {
	if (!Particles::attrs.size())
		RETERR("No attribute available!");
	auto& ov = ((DmScript_I*)script.get())->outputVs;
	ov[0].val.p = Particles::attrs[attrId]->Get(
		(!AnWeb::execFrame)? Particles::anim.currentFrame : AnWeb::execFrame-1).data();
}

void Node_GetAttribute::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
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