#include "node_addbond.h"
#ifndef IS_ANSERVER
#include "md/Particles.h"
#include "md/ParMenu.h"
#include "ui/icons.h"
#include "ui/ui_ext.h"
#endif

Node_AddBond::Node_AddBond() : AnNode(new DmScript(sig)) {
	title = "Extra Bonds";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
	canTile = false;
	inputR.resize(2);
	script->invars.push_back(std::pair<string, string>("pair counts", "list(1i)"));
	script->invars.push_back(std::pair<string, string>("bonds", "list(1i)"));

	Particles::anim.conns2.resize(1);
	Particles::particles_Conn2.resize(1);
	animId = 0;
}

Node_AddBond::~Node_AddBond() {
	auto& bk2 = Particles::anim.conns2[animId];
	delete[](bk2.first);
	for (uint i = 0; i < Particles::anim.frameCount; i++)
		delete[](bk2.second[i]);
	delete[](bk2.second);
	Particles::particles_Conn2.erase(Particles::particles_Conn2.begin() + animId);
	Particles::anim.conns2.erase(Particles::anim.conns2.begin() + animId);
}

void Node_AddBond::Execute() {
	if (!inputR[0].first || !inputR[1].first) return;
	CVar& cv1 = inputR[0].first->conV[inputR[0].second];
	CVar& cv2 = inputR[1].first->conV[inputR[1].second];
	if (*cv1.dimVals[0] != Particles::anim.frameCount) return;
	auto& c2 = Particles::anim.conns2[animId];
	uint off = 0;
	for (uint i = 0; i < Particles::anim.frameCount; i++) {
		c2.first[i] = (*((int**)cv1.value))[i];
		c2.second[i] = *((Int2**)cv2.value) + off;
		off += c2.first[i];
	}
}

void Node_AddBond::DrawSettings(float& off) {
	float off0 = off;
	auto& cn = Particles::particles_Conn2[0];
	ParMenu::DrawConnMenu(cn, pos.x, off, width);
	settSz = (int)(off - off0);
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

void Node_AddBond::LoadOut(const string& path) {
	Execute();
}