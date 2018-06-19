#include "node_addbond.h"
#ifndef IS_ANSERVER
#include "md/Particles.h"
#include "ui/icons.h"
#endif

Node_AddBond::Node_AddBond() : AnNode(new DmScript()) {
	title = "Extra Bonds";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
	canTile = false;
	inputR.resize(2);
	script->name = ".ABnd";
	script->invars.push_back(std::pair<string, string>("pair counts", "ilist(1)"));
	script->invars.push_back(std::pair<string, string>("bonds", "ilist(1)"));

	Particles::anim.conns2.resize(1);
	Particles::particles_Conn2.resize(1);
	auto& bk2 = Particles::anim.conns2.back();
	bk2.first = new uint[140]{};
	bk2.second = new Int2*[140];
	animId = 0;
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

float Node_AddBond::DrawSide() {
	auto f = AnNode::DrawSide();
	auto& c2 = Particles::particles_Conn2[animId];
	if (Engine::Button(pos.x + width - 17, pos.y, 16, 16, c2.visible? Icons::visible : Icons::hidden, white(0.8f), white(), white(1, 0.5f))) {
		c2.visible = !c2.visible;
	}
	if (Engine::Button(pos.x + width - 50, pos.y, 16, 16, Icons::dm_line, (!c2.drawMode)? yellow() : white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		c2.drawMode = 0;
		Scene::dirty = true;
	}
	if (Engine::Button(pos.x + width - 34, pos.y, 16, 16, Icons::dm_stick, (!c2.drawMode)? white(0.8f) : yellow(), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		c2.drawMode = 1;
		Scene::dirty = true;
	}
	return f;
}

void Node_AddBond::LoadOut(const string& path) {
	Execute();
}