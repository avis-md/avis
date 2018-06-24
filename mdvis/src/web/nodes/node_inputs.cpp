#include "../anweb.h"
#ifndef IS_ANSERVER
#include "md/Particles.h"
#endif

Node_Inputs::Node_Inputs() : AnNode(new DmScript()) {
	DmScript* scr = (DmScript*)script;
	script->name = ".in";
	
	title = "All Particles";
	titleCol = Vec3(0.3f, 0.3f, 0.5f);
	canTile = true;
	auto v = std::pair<string, string>();
	v.second = "list(2)";
	outputR.resize(4);
	scr->outvars.resize(4, v);
	scr->outvars[0].first = "positions";
	scr->outvars[1].first = "velocities";
	scr->outvars[2].first = "positions (all)";
	scr->outvars[3].first = "velocities (all)";
	scr->outvars[2].second = scr->outvars[3].second = "list(3)";

	conV.resize(4);
	auto& poss = conV[0];
	poss.dimVals.resize(2);
	poss.type = AN_VARTYPE::LIST;
#ifndef IS_ANSERVER
	poss.dimVals[0] = (int*)&Particles::particleSz;
#endif
	poss.dimVals[1] = new int(3);
	
	conV[1] = conV[2] = poss;
	auto& posa = conV[2];
	posa.dimVals.resize(3);
	posa.dimVals[2] = posa.dimVals[1];
	posa.dimVals[1] = posa.dimVals[0];
#ifndef IS_ANSERVER
	posa.dimVals[0] = (int*)&Particles::anim.frameCount;
#endif
	conV[3] = conV[2];
}

void Node_Inputs::Draw() {
#ifndef IS_ANSERVER
	auto cnt = 4;
	this->pos = pos;
	Engine::DrawQuad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.0f : 0.7f));
	UI::Label(pos.x + 2, pos.y + 1, 12, title, white());
	Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.7f, 0.25f));
	float y = pos.y + 20;
	for (int i = 0; i < cnt; i++, y += 17) {
		if (!AnWeb::selConnNode || ((!AnWeb::selConnIdIsOut) && (AnWeb::selConnNode != this))) {
			if (Engine::Button(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
				if (!AnWeb::selConnNode) {
					AnWeb::selConnNode = this;
					AnWeb::selConnId = i;
					AnWeb::selConnIdIsOut = true;
					AnWeb::selPreClear = false;
				}
				else {
					ConnectTo(i, AnWeb::selConnNode, AnWeb::selConnId);
					AnWeb::selConnNode = nullptr;
				}
			}
		}
		else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && AnWeb::selConnIdIsOut) {
			Engine::DrawLine(Vec2(pos.x + width, y + 8), Input::mousePos, white(), 1);
			UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open);
		}
		else {
			UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open, red(0.3f));
		}

		//UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open);
		UI::font->Align(ALIGN_TOPRIGHT);
		UI::Label(pos.x + width - 10, y, 12, script->outvars[i].first, white());
		UI::font->Align(ALIGN_TOPLEFT);
		UI::Label(pos.x + 2, y, 12, script->outvars[i].second, white(0.3f), width * 0.67f - 6);
	}
#endif
}

void Node_Inputs::Execute() {
#ifndef IS_ANSERVER
	conV[0].value = &Particles::particles_Pos;
	conV[1].value = &Particles::particles_Vel;
	conV[2].value = Particles::anim.poss;
	conV[3].value = Particles::anim.vels;
#endif
}

void Node_Inputs::SaveIn(const string& path) {
	Execute();
	string nm = script->name;
	//std::replace(nm.begin(), nm.end(), '/', '_');
	std::ofstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		for (uint i = 0; i < 4; i++)
			conV[i].Write(strm);
	}
}

void Node_Inputs::LoadIn(const string& path) {
	string nm = script->name;
	//std::replace(nm.begin(), nm.end(), '/', '_');
	std::ifstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		for (uint i = 0; i < 4; i++)
			conV[i].Read(strm);
	}
	else {
		Debug::Error("Node", "cannot open input file!");
	}
}

Node_Inputs_ActPar::Node_Inputs_ActPar() : Node_Inputs() {
	title = "Selected Particles";
	script->name = ".insel";

	for (uint i = 0; i < 4; i++)
		conV[i].dimVals[0] = new int(0);
}

void Node_Inputs_ActPar::Execute() {

}