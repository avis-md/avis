#include "../anweb.h"
#ifndef IS_ANSERVER
#include "md/Particles.h"
#include "ui/ui_ext.h"
#endif

Node_Inputs::Node_Inputs() : AnNode(new DmScript(sig)) {
	DmScript* scr = (DmScript*)script;
	script->desc = "Particle coordinates and trajectory";
	script->descLines = 2;
	
	title = "All Particles";
	titleCol = Vec3(0.3f, 0.3f, 0.5f);
	canTile = true;
	auto v = std::pair<string, string>();
	v.second = "list(2d)";
	outputR.resize(8);
	scr->outvars.resize(8, v);
	scr->outvars[0].first = "atom count";
	scr->outvars[1].first = "frame count";
	scr->outvars[2].first = "frame";
	scr->outvars[0].second = scr->outvars[1].second
		= scr->outvars[2].second = "int";
	scr->outvars[3].first = "positions";
	scr->outvars[4].first = "velocities";
	scr->outvars[5].first = "types";
	scr->outvars[5].second = "list(1s)";
	scr->outvars[6].first = "positions (all)";
	scr->outvars[7].first = "velocities (all)";
	scr->outvars[6].second = scr->outvars[7].second = "list(3d)";
	
	conV.resize(8);
	
	auto& posc = conV[0];
	posc.type = AN_VARTYPE::INT;
#ifndef IS_ANSERVER
	posc.value = &Particles::particleSz;
#endif

	conV[1] = conV[2] = posc;
#ifndef IS_ANSERVER
	conV[1].value = &Particles::anim.frameCount;
	conV[2].value = &Particles::anim.activeFrame;
#endif

	auto& poss = conV[3];
	poss.dimVals.resize(2);
	poss.type = AN_VARTYPE::LIST;
#ifndef IS_ANSERVER
	poss.dimVals[0] = (int*)&Particles::particleSz;
	poss.value = &Particles::particles_Pos;
#endif
	poss.dimVals[1] = new int(3);
	
	conV[4] = poss;
#ifndef IS_ANSERVER
	conV[4].value = &Particles::particles_Vel;
#endif
	
	auto& post = conV[5];
	post.dimVals.resize(1);
#ifndef IS_ANSERVER
	post.dimVals[0] = (int*)&Particles::particleSz;
	post.value = &Particles::particles_Typ;
#endif

	auto& posa = conV[6];
	posa.dimVals.resize(3);
	posa.dimVals[2] = posa.dimVals[1];
	posa.dimVals[1] = posa.dimVals[0];
#ifndef IS_ANSERVER
	posa.dimVals[0] = (int*)&Particles::anim.frameCount;
	posa.dimVals[1] = (int*)&Particles::particleSz;
	posa.dimVals[2] = poss.dimVals[1];
#endif
	conV[7] = posa;
}

void Node_Inputs::Execute() {
#ifndef IS_ANSERVER
	conV[6].value = Particles::anim.poss;
	conV[7].value = Particles::anim.vels;
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
	script->name = sig;
	script->desc = "Particle data for all frames";
	script->descLines = 1;

	for (uint i = 0; i < 4; i++)
		conV[i].dimVals[0] = new int(0);
}

void Node_Inputs_ActPar::Execute() {

}

Node_Inputs_SelPar::Node_Inputs_SelPar() : Node_Inputs() {
	title = "Particles of";
	script->name = sig;
	script->desc = "Particle data of Name/ID";
	script->descLines = 1;

	for (uint i = 0; i < 4; i++)
		conV[i].dimVals[0] = new int(0);
}

void Node_Inputs_SelPar::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	Engine::DrawQuad(pos.x, off, width, 35, bgCol);
	string nms[] = { "ResNm", "ResID", "AtomID" };
	string nmfs[] = { "Residue Name", "Residue ID", "Atom ID" };
	UI2::Switch(pos.x + 2, off + 1, width - 2, "type", 3, nms, (int&)type);
	UI::Label(pos.x + 2, off + 18, 12, nmfs[(int)type], white());
	string tv = (type == SELTYPE::RSL)? tv_resNm : 
		(type == SELTYPE::RES)? std::to_string(tv_resId) : 
			std::to_string(tv_atomId);
	string tv2 = (type == SELTYPE::RSL)? tv_resNm : 
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