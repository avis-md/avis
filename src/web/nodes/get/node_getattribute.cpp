// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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