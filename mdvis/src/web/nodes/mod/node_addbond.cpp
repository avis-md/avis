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
	INODE_TITLE(NODE_COL_MOD);
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("bonds", "list(2i)"));

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
	CVar& cv = inputR[0].getconv();
	auto d1 = *cv.dimVals[1];
	if (d1 != 2) RETERR("dimension 1 is " + std::to_string(d1) + ", expected 2!");
	auto& c = Particles::anim.conns2[animId];
	c.resize(Particles::anim.frameCount);
	auto& c2 = c[AnWeb::realExecFrame];
	c2.count = *cv.dimVals[0];
	c2.ids.resize(c2.count);
	memcpy(&c2.ids[0], *((Int2**)cv.value), c2.count * sizeof(Int2));
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
	}
	if (Engine::Button(pos.x + width - 34, pos.y, 16, 16, Icons::down, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		showSett = !showSett;
	}
	return f;
}

void Node_AddBond::LoadOut(const std::string& path) {
	Execute();
}