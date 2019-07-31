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

#include "node_setattribute.h"
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "web/anweb.h"

INODE_DEF(__("Set Attribute"), SetAttribute, SET)

Node_SetAttribute::Node_SetAttribute() : INODE_INIT, attrId(0), _attrId(-1), attrSz(-1), di(&attrId, nullptr), timed(true) {
	INODE_TITLE(NODE_COL_IO);
	INODE_SINIT(
		scr->AddInput(_("values"), AN_VARTYPE::ANY, 1);
	);
}

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

void Node_SetAttribute::Execute() {
	auto& ir = inputR[0];
    if (!ir.first) return;
	if (!Particles::attrs.size()) {
		RETERR("No attribute available!");
	}
	auto sz = ir.getdim(0);
	if (sz != Particles::particleSz)
		RETERR("Attribute must be for each atom!");
	auto src = *((void**)ir.getval(ANVAR_ORDER::C));
	auto prm = Particles::attrs[attrId + Particles::readonlyAttrCnt];
	prm->timed = (AnWeb::execFrame > 0 && timed);
	auto& tar = prm->Get(AnWeb::realExecFrame);
	tar.resize(sz);
	switch (ir.getvar().typeName[6]) {
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
		RETERR("Unexpected data type " + ir.getvar().typeName + "!");
	}
	prm->dirty = true;
	if (prm->timed) prm->Set((uint)(AnWeb::realExecFrame));
}

void Node_SetAttribute::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	if (attrSz != Particles::attrs.size()) {
		attrSz = Particles::attrs.size();
		attrs.resize(attrSz - Particles::readonlyAttrCnt);
		if (attrId >= attrs.size()) {
			attrId = (uint)std::max((int)attrs.size()-1, 0);
		}
		attrs.push_back("<create new>");
		attrs.push_back("");
		di.list = &attrs[0];
	}
	for (uint a = Particles::readonlyAttrCnt; a < attrSz; ++a) {
		attrs[a - Particles::readonlyAttrCnt] = Particles::attrNms[a];
	}
	UI2::Dropdown(pos.x + 2, off, width - 4, "Attribute", di);
	if (attrId != _attrId) {
		_attrId = attrId;
		if (attrId == attrs.size()-2) {
			Particles::AddAttr();
		}
	}
	off += 17;
	UI2::Toggle(pos.x + 2, off, width - 4, "Animated", timed);
	off += 18;
}

void Node_SetAttribute::Save(XmlNode* n) {
	n->addchild("id", std::to_string(attrId));
	n->addchild("timed", timed? "1" : "0");
}

void Node_SetAttribute::Load(XmlNode* n2) {
	for (auto& n : n2->children) {
		if (n.name == "id") attrId = TryParse(n.value, 0U);
		else if (n.name == "timed") timed = (n.value == "1");
	}
	while (Particles::attrs.size() - Particles::readonlyAttrCnt <= attrId) {
		Particles::AddAttr();
	}
}

void Node_SetAttribute::LoadOut(const std::string& path) {
	Execute();
}