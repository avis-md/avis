#include "../anweb.h"
#ifndef IS_ANSERVER
#include "md/Particles.h"
#include "ui/ui_ext.h"
#endif

Node_Inputs::Node_Inputs() : AnNode(new DmScript(sig)) {
	DmScript* scr = (DmScript*)script;
	script->desc = "Particle coordinates and trajectory";
	script->descLines = 2;
	
	title = "Particle Data";
	titleCol = Vec3(0.3f, 0.3f, 0.5f);
	canTile = true;
	auto v = std::pair<std::string, std::string>();
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
	
	CVar cv;
	cv.stride = 8;
	conV.resize(8, cv);
	
	auto& posc = conV[0];
	posc.type = AN_VARTYPE::INT;
#ifndef IS_ANSERVER
	posc.value = &Particles::particleSz;
#endif
	posc.stride = 4;

	conV[1] = conV[2] = posc;
#ifndef IS_ANSERVER
	conV[1].value = &Particles::anim.frameCount;
	conV[2].value = &Particles::anim.activeFrame;
#endif

	auto& poss = conV[3];
	poss.dimVals.resize(2);
	poss.data.dims.resize(1, 3);
	poss.type = AN_VARTYPE::LIST;
#ifndef IS_ANSERVER
	poss.dimVals[0] = (int*)&Particles::particleSz;
	poss.value = &Particles::particles_Pos;
#endif
	poss.dimVals[1] = &poss.data.dims[0];
	
	auto& vels = conV[4] = poss;
#ifndef IS_ANSERVER
	vels.value = &Particles::particles_Vel;
#endif
	vels.dimVals[1] = &vels.data.dims[0];
	
	auto& post = conV[5];
	post.dimVals.resize(1);
#ifndef IS_ANSERVER
	post.dimVals[0] = (int*)&Particles::particleSz;
	post.value = &Particles::particles_Typ;
#endif
	post.stride = 2;

	auto& posa = conV[6];
	posa.dimVals.resize(3);
	posa.data.dims.resize(1);
	//posa.dimVals[2] = posa.dimVals[1];
	//posa.dimVals[1] = posa.dimVals[0];
#ifndef IS_ANSERVER
	posa.dimVals[0] = (int*)&Particles::anim.frameCount;
	posa.dimVals[1] = (int*)&Particles::particleSz;
	posa.dimVals[2] = &posa.data.dims[0];
#endif
	conV[7] = posa;

	for (int a = 0; a < 8; a++) {
		conV[a].typeName = scr->outvars[a].second;
	}
}

void Node_Inputs::Execute() {
#ifndef IS_ANSERVER
	conV[6].value = Particles::anim.poss;
	conV[7].value = Particles::anim.vels;
#endif
}

void Node_Inputs::SaveIn(const std::string& path) {
	Execute();
	std::string nm = script->name;
	//std::replace(nm.begin(), nm.end(), '/', '_');
	std::ofstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		for (uint i = 0; i < 4; i++)
			conV[i].Write(strm);
	}
}

void Node_Inputs::LoadIn(const std::string& path) {
	std::string nm = script->name;
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