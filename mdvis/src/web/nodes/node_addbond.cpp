#include "node_addbond.h"
#ifndef IS_ANSERVER
#include "md/Particles.h"
#include "ui/icons.h"
#include "ui/ui_ext.h"
#endif

Node_AddBond::Node_AddBond() : AnNode(new DmScript(".ABnd")) {
	title = "Extra Bonds";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
	canTile = false;
	inputR.resize(2);
	script->invars.push_back(std::pair<string, string>("pair counts", "list(1i)"));
	script->invars.push_back(std::pair<string, string>("bonds", "list(1i)"));

	Particles::anim.conns2.resize(1);
	Particles::particles_Conn2.resize(1);
	auto& bk2 = Particles::anim.conns2.back();
	bk2.first = new uint[140]{};
	bk2.second = new Int2*[140]{};
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
#define SV(v) auto _ ## v = cn.v
#define CP(v) if (_ ## v != cn.v) { cn.v = _ ## v; Scene::dirty = true; }
	auto& cn = Particles::particles_Conn2[0];
	settSz = 17 * (((!cn.drawMode)? (cn.dashed? 5 : 3) : 2) + (cn.usecol? 2 : 1)) + 2;
	Engine::DrawQuad(pos.x, off, width, (float)settSz, white(0.7f, 0.15f));
	off++;
	bool dl = !!cn.drawMode;
	UI2::Toggle(pos.x + 2, off, width - 4, "Solid", dl);
	if (dl != !!cn.drawMode) {
		cn.drawMode = dl? 1 : 0;
		Scene::dirty = true;
	}
	off += 17;
	if (dl) {
		auto _scale = UI2::Slider(pos.x + 2, off, width - 4, "Thickness", 0, 1, cn.scale);
		off += 17;
		CP(scale);
	}
	else {
		SV(dashed);
		auto _line_sc = UI2::Slider(pos.x + 2, off, width - 4, "Thickness", 1, 5, cn.line_sc);
		off += 17;
		UI2::Toggle(pos.x + 2, off, width - 4, "Dashed lines", _dashed);
		off += 17;
		if (_dashed) {
			auto _line_sp = UI2::Slider(pos.x + 2, off, width - 4, "Spacing", 0, 1, cn.line_sp);
			off += 17;
			auto _line_rt = UI2::Slider(pos.x + 2, off, width - 4, "Ratio", 0, 1, cn.line_rt);
			off += 17;
			CP(line_sp); CP(line_rt);
		}
		CP(dashed); CP(line_sc);
	}
	SV(usecol);
	UI2::Toggle(pos.x + 2, off, width - 4, "Custom Colors", _usecol);
	off += 17;
	if (_usecol) {
		SV(col);
		UI2::Color(pos.x + 2, off, width - 4, "Color", _col);
		off += 18;
		CP(col);
	}
	else off++;
	CP(usecol);
#undef SV
#undef CP
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