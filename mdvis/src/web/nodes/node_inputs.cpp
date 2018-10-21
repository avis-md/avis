#include "../anweb.h"
#ifndef IS_ANSERVER
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "vis/pargraphics.h"
#endif

uint Node_Inputs::frame = 0, Node_Inputs::parcount = 0;

Node_Inputs::Node_Inputs() : AnNode(new DmScript(sig)), filter(0) {
	DmScript* scr = (DmScript*)script;
	script->desc = R"(Particle coordinates and trajectory
 positions: [atomId, xyz]
 velocities: [atomId, xyz]
 positions (all): [frame, atomId, xyz]
 velocities (all): [frame, atomId, xyz]
 types: [atomid]
 * type is the ascii of the atom name,
   so C is 67, O is 79 etc.)";
	script->descLines = 8;
	
	title = "Particle Data";
	titleCol = NODE_COL_IO;
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
	poss.dimVals[0] = (int*)&parcount;
#endif
	poss.dimVals[1] = &poss.data.dims[0];
	
	auto& vels = conV[1] = poss;
	vels.dimVals[1] = &vels.data.dims[0];

	auto& posa = conV[2];
	posa.dimVals.resize(3);
	posa.data.dims.resize(1, 3);
#ifndef IS_ANSERVER
	posa.dimVals[0] = (int*)&Particles::anim.frameCount;
	posa.dimVals[1] = (int*)&parcount;
#endif
	posa.dimVals[2] = &posa.data.dims[0];
	conV[3] = posa;

	auto& post = conV[4];
	post.dimVals.resize(1);
#ifndef IS_ANSERVER
	post.dimVals[0] = (int*)&parcount;
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
	di.target = &filter;
	UI2::Dropdown(pos.x + 5, off, width - 10, "Filter", di);
	off += 17;
}

void Node_Inputs::Execute() {
#ifndef IS_ANSERVER
	bool setpos = outputR[0].size() > 0;
	bool setvel = outputR[1].size() > 0;
	bool settyp = outputR[4].size() > 0;

	glm::dvec3* pos, *vel;
	if (!Particles::anim.poss.size()) {
		pos = Particles::poss;
		vel = Particles::vels;
	}
	else {
		pos = &Particles::anim.poss[frame][0];
		vel = &Particles::anim.vels[frame][0];
	}
	if (!filter) {
		parcount = Particles::particleSz;
		conV[0].data.val.arr.p = pos;
		conV[0].value = &conV[0].data.val.arr.p;
		conV[1].data.val.arr.p = vel;
		conV[1].value = &conV[1].data.val.arr.p;
		conV[4].value = &Particles::types;
	}
	else {
		if (setpos) {
			vpos.clear();
			vpos.reserve(Particles::particleSz);
		}
		if (setpos) {
			vvel.clear();
			vvel.reserve(Particles::particleSz);
		}
		if (setpos) {
			vtyp.clear();
			vtyp.reserve(Particles::particleSz);
		}
		int off = 0;
		if ((filter & (int)FILTER::VIS) > 0) {
			for (auto& dl : ParGraphics::drawLists) {
				vpos.resize(off + dl.second.first);
				memcpy(&vpos[off], pos + dl.first, dl.second.first * sizeof(glm::dvec3));
				if (setvel) {
					vvel.resize(off + dl.second.first);
					memcpy(&vvel[off], vel + dl.first, dl.second.first * sizeof(glm::dvec3));
				}
				if (settyp) {
					vtyp.resize(off + dl.second.first);
					memcpy(&vtyp[off], &Particles::types[dl.first], dl.second.first * sizeof(short));
				}
				off += dl.second.first;
			}
			parcount = (uint)vpos.size();
		}
		if ((filter & (int)FILTER::CLP) > 0 && ParGraphics::clippingType != ParGraphics::CLIPPING::NONE) {
			std::vector<glm::dvec3> tpos; std::swap(tpos, vpos); vpos.reserve(parcount);
			std::vector<glm::dvec3> tvel; if (setvel) { std::swap(tvel, vvel); vvel.reserve(parcount); }
			std::vector<short> ttyp; if (settyp) { std::swap(ttyp, vtyp); vtyp.reserve(parcount); }
			for (int a = 0; a < parcount; a++) {
				auto& pos = tpos[a];
				bool clipped = false;
				for (int c = 0; c < 6; c++) {
					if (glm::dot((Vec3)pos, (Vec3)ParGraphics::clippingPlanes[c]) > ParGraphics::clippingPlanes[c].w) {
						clipped = true;
						break;
					}
				}
				if (clipped) continue;
				vpos.push_back(pos);
				if (setvel) vvel.push_back(tvel[a]);
				if (setvel) vtyp.push_back(ttyp[a]);
			}
			parcount = (uint)vpos.size();
		}
		poss = &vpos[0][0];
		vels = &vvel[0][0];
		typs = &vtyp[0];
		conV[0].value = &poss;
		conV[1].value = &vels;
		conV[4].value = &typs;
	}
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