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

#include "node_addbond.h"
#include "web/anweb.h"
#ifndef IS_ANSERVER
#include "md/particles.h"
#include "md/parmenu.h"
#include "ui/icons.h"
#include "ui/ui_ext.h"
#endif

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

INODE_DEF(__("Extra Bonds"), AddBond, GEN)

Node_AddBond::Node_AddBond() : INODE_INIT {
	INODE_TITLE(NODE_COL_MOD)
	INODE_SINIT(
		scr->AddInput(_("bonds"), AN_VARTYPE::INT, 2);
	);

	Particles::anim.conns2.resize(1);
	Particles::particles_Conn2.resize(1);
	animId = 0;
	auto& c2 = Particles::anim.conns2[animId];
	c2.clear();
}

Node_AddBond::~Node_AddBond() {
	Particles::particles_Conn2.erase(Particles::particles_Conn2.begin() + animId);
	Particles::anim.conns2.erase(Particles::anim.conns2.begin() + animId);
}

void Node_AddBond::Execute() {
	//
	if (!Particles::anim.conns2.size()) {
		Particles::anim.conns2.resize(1);
		Particles::particles_Conn2.resize(1);
		animId = 0;
		auto& c2 = Particles::anim.conns2[animId];
		c2.clear();
	}

	if (!inputR[0].first) return;
	auto& ir = inputR[0];
	auto d1 = ir.getdim(1);
	if (d1 != 2) RETERR("dimension 1 is " + std::to_string(d1) + ", expected 2!");
	auto& c = Particles::anim.conns2[animId];
	c.resize(Particles::anim.frameCount);
	auto& c2 = c[AnWeb::realExecFrame];
	c2.count = ir.getdim(0);
	c2.ids.resize(c2.count);
	memcpy(&c2.ids[0], *((Int2**)ir.getval(ANVAR_ORDER::C)), c2.count * sizeof(Int2));
}

void Node_AddBond::DrawSettings(float& off) {
	auto& cn = Particles::particles_Conn2[0];
	ParMenu::DrawConnMenu(cn, pos.x, off, width);
}

float Node_AddBond::DrawSide() {
	auto f = AnNode::DrawSide();
	auto& c2 = Particles::particles_Conn2[animId];
	if (Engine::Button(pos.x + width - 17, pos.y, 16, 16, c2.visible? Icons::visible : Icons::hidden, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		c2.visible = !c2.visible;
		Scene::dirty = true;
	}
	if (Engine::Button(pos.x + width - 34, pos.y, 16, 16, Icons::down, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		showSett = !showSett;
	}
	return f;
}

void Node_AddBond::LoadOut(const std::string& path) {
	Execute();
}