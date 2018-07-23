#include "../anweb.h"
#ifndef IS_ANSERVER
#include "md/Particles.h"
#include "ui/ui_ext.h"
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
	float y = pos.y + 16;
	DrawHeader(y);
	Engine::DrawQuad(pos.x, y, width, 3.0f + 17 * cnt, white(0.7f, 0.25f));
	y += 4;
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
	script->name = ".inact";

	for (uint i = 0; i < 4; i++)
		conV[i].dimVals[0] = new int(0);
}

void Node_Inputs_ActPar::Execute() {

}

Node_Inputs_SelPar::Node_Inputs_SelPar() : Node_Inputs() {
	title = "Particles of";
	script->name = ".insel";

	for (uint i = 0; i < 4; i++)
		conV[i].dimVals[0] = new int(0);
}

void Node_Inputs_SelPar::DrawHeader(float& off) {
	Engine::DrawQuad(pos.x, off, width, 35, white(0.7f, 0.25f));
	string nms[] = { "ResNm", "ResID", "AtomID" };
	string nmfs[] = { "Residue Name", "Residue ID", "Atom ID" };
	UI2::Switch(pos.x + 2, off + 1, width - 2, "type", 3, nms, (int&)type);
	UI::Label(pos.x + 2, off + 18, 12, nmfs[(int)type], white());
	string tv2 = (type == SELTYPE::RSL)? tv_resNm : 
		(type == SELTYPE::RES)? std::to_string(tv_resId) : 
			std::to_string(tv_atomId);
	string tv = (type == SELTYPE::RSL)? tv_resNm : 
		(type == SELTYPE::RES)? std::to_string(tv_resId) + " (" + tv_resNm + ")" : 
			std::to_string(tv_atomId) + " (" + string(Particles::particles_Name, PAR_MAX_NAME_LEN) + ")";
	tv = UI::EditText(pos.x + width / 2, off + 18, width/2 - 18, 16, 12, white(1, 0.3f), tv, true, white(), 0, tv2);
	if (Engine::Button(pos.x + width - 17, off + 18, 16, 16, white(1, 0.5f)) == MOUSE_RELEASE) {
		Popups::type = (POPUP_TYPE)((int)POPUP_TYPE::RESNM + (int)type);
		Popups::data = (type == SELTYPE::RSL)? (void*)&tv_resNm : (void*)((type == SELTYPE::RES)? &tv_resId : &tv_atomId);
	}
	if (type == SELTYPE::RSL) tv_resNm = tv;
	else if (type == SELTYPE::RES) tv_resId = TryParse(tv, 0U);
	else tv_atomId = TryParse(tv, 0U);
	off += 35;
}

void Node_Inputs_SelPar::Execute() {

}