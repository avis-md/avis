#include "../anweb.h"
#ifndef IS_ANSERVER
#include "md/Particles.h"
#include "ui/ui_ext.h"
#endif

uint Node_Inputs::frame = 0;

Node_Inputs::Node_Inputs() : AnNode(new DmScript(sig)), filter(0) {
	DmScript* scr = (DmScript*)script;
	script->desc = "Particle coordinates and trajectory";
	script->descLines = 2;
	
	title = "Particle Data";
	titleCol = Vec3(0.3f, 0.3f, 0.5f);
	canTile = true;
	
	const std::string vars[] = {
		"positions", "list(2d)",
		"velocities", "list(2d)",
		"positions (all)", "list(3d)",
		"velocities (all)", "list(3d)",
		"types", "list(1s)"
	};
	const int sz = sizeof(vars) / sizeof(vars[0]) / 2;

	outputR.resize(sz);
	script->outvars.resize(sz);
	conV.resize(sz);

	for (int a = 0; a < sz; a++) {
		script->outvars[a] = std::pair<std::string, std::string>(vars[a * 2], vars[a * 2 + 1]);
		conV[a].typeName = vars[a * 2 + 1];
	}

	auto& poss = conV[0];
	poss.dimVals.resize(2);
	poss.data.dims.resize(1, 3);
	poss.type = AN_VARTYPE::LIST;
#ifndef IS_ANSERVER
	poss.dimVals[0] = (int*)&Particles::particleSz;
#endif
	poss.dimVals[1] = &poss.data.dims[0];
	
	auto& vels = conV[1] = poss;
	vels.dimVals[1] = &vels.data.dims[0];

	auto& posa = conV[2];
	posa.dimVals.resize(3);
	posa.data.dims.resize(1, 3);
#ifndef IS_ANSERVER
	posa.dimVals[0] = (int*)&Particles::anim.frameCount;
	posa.dimVals[1] = (int*)&Particles::particleSz;
#endif
	posa.dimVals[2] = &posa.data.dims[0];
	conV[3] = posa;

	auto& post = conV[4];
	post.dimVals.resize(1);
#ifndef IS_ANSERVER
	post.dimVals[0] = (int*)&Particles::particleSz;
#endif
	post.stride = 2;

	for (int a = 0; a < sz; a++) {
		conV[a].typeName = scr->outvars[a].second;
	}
}

void Node_Inputs::DrawHeader(float& off) {
	AnNode::DrawHeader(off);

	UI::Quad(pos.x, off, width, 17, bgCol);
	static std::string ss[] = { "Visible", "Clipped", "" };
	static Popups::DropdownItem di(&filter, &ss[0], true);
	UI2::Dropdown(pos.x + 5, off, width - 10, "Filter", di);
	off += 17;
}

void Node_Inputs::Execute() {
#ifndef IS_ANSERVER
	conV[0].value = &Particles::anim.poss[frame];
	conV[1].value = &Particles::anim.vels[frame];
	conV[2].value = Particles::anim.poss;
	conV[3].value = Particles::anim.vels;
	conV[4].value = &Particles::particles_Typ;
#endif
}

void Node_Inputs::SaveIn(const std::string& path) {
	Execute();
	std::string nm = script->name;
	std::ofstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		for (uint i = 0; i < 4; i++)
			conV[i].Write(strm);
	}
}

void Node_Inputs::LoadIn(const std::string& path) {
	std::string nm = script->name;
	std::ifstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		for (uint i = 0; i < 4; i++)
			conV[i].Read(strm);
	}
	else {
		Debug::Error("Node", "cannot open input file!");
	}
}