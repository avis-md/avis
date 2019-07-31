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

#include "node_showrange.h"
#include "md/particles.h"
#include "vis/pargraphics.h"
#include "ui/ui_ext.h"

INODE_DEF(__("Show Range"), ShowRange, SET)

Node_ShowRange::Node_ShowRange() : INODE_INIT, invert(false), rMin(0), rMax(1) {
	INODE_TITLE(NODE_COL_MOD);
	INODE_SINIT(
		scr->AddInput(_("values"), AN_VARTYPE::DOUBLE, 1);
	);
}

void Node_ShowRange::Execute() {
    if (!inputR[0].first) return;
	auto& ir = inputR[0];
	if (ir.getdim(0) != Particles::particleSz) return;

    double* vals = *(double**)ir.getval(ANVAR_ORDER::C);

#pragma omp parallel for
    for (int a = 0; a < Particles::particleSz; ++a) {
        auto& v = vals[a];
        if (v >= rMin && v <= rMax)
            Particles::visii[a] = !invert;
        else
            Particles::visii[a] = invert;
    }
    Particles::visDirty = true;
	canReset = true;
}

void Node_ShowRange::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI2::Toggle(pos.x + 2, off, width - 4, "invert", invert);
    title = invert? "Hide Range" : "Show Range";
    off += 18;
    auto s = std::to_string(rMin);
    s = UI2::EditText(pos.x + 2, off, (uint)width - 4, "min", s);
    rMin = TryParse(s, 0.f);
    s = std::to_string(rMax);
    s = UI2::EditText(pos.x + 2, off + 17, (uint)width - 4, "max", s);
    rMax = TryParse(s, 0.f);
	off += 35;
}

void Node_ShowRange::DrawFooter(float& off) {
	if (canReset) {
		if (Engine::Button(pos.x + 2, off, width - 4, 16, white(1, 0.4f), "Reset", 12, white(), true) == MOUSE_RELEASE) {
#pragma omp parallel for
			for (int a = 0; a < Particles::particleSz; ++a) {
				Particles::visii[a] = true;
			}
			Particles::visDirty = true;
			canReset = false;
		}
		off += 17;
	}
}

void Node_ShowRange::Save(XmlNode* n) {
	n->addchild("invert", invert? "1" : "0");
    n->addchild("min", std::to_string(rMin));
    n->addchild("max", std::to_string(rMax));
}

void Node_ShowRange::Load(XmlNode* n2) {
	for (auto& n : n2->children) {
		if (n.name == "invert") invert = (n.value == "1");
		else if (n.name == "min") rMin = TryParse(n.value, 0.0f);
		else if (n.name == "max") rMax = TryParse(n.value, 1.0f);
	}
}

void Node_ShowRange::LoadOut(const std::string& path) {
    Execute();
}