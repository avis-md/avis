#include "node_inputs.h"
#include "web/anweb.h"
#ifndef IS_ANSERVER
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "vis/pargraphics.h"
#endif

INODE_DEF(__("Particle Data"), Inputs, GET)

uint Node_Inputs::parcount = 0;

//enable saveconv?
Node_Inputs::Node_Inputs() : INODE_INITF(AN_FLAG_NOSAVECONV), filter(0) {
	INODE_TITLE(NODE_COL_IO);

	scr.desc = R"(Particle coordinates and trajectory
positions: [atomId, xyz]
velocities: [atomId, xyz]
positions (all): [frame, atomId, xyz]
velocities (all): [frame, atomId, xyz]
types: [atomid]
* type is the ascii of the atom name,
so C is 67, O is 79 etc.)";
	script->descLines = 8;

	AddOutput(CVar(_("positions"), 'd', 2, { (int*)&parcount, nullptr }, { 3 }));
	scr.AddOutput(conV.back());

	AddOutput(CVar(_("velocities"), 'd', 2, { (int*)&parcount, nullptr }, { 3 }));
	scr.AddOutput(conV.back());

	AddOutput(CVar(_("types"), 's', 1, { (int*)&parcount }));
	scr.AddOutput(conV.back());
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
	bool settyp = outputR[2].size() > 0;
	setpos |= (setvel || settyp) && ((filter & (int)FILTER::CLP) > 0);

	glm::dvec3* pos, *vel;
	if (!Particles::anim.poss.size()) {
		pos = Particles::poss;
		vel = Particles::vels;
	}
	else {
		pos = &Particles::anim.poss[AnWeb::realExecFrame][0];
		vel = &Particles::anim.vels[AnWeb::realExecFrame][0];
	}
	if (!filter) {
		parcount = Particles::particleSz;
		conV[0].data.val.arr.p = pos;
		conV[0].value = &conV[0].data.val.arr.p;
		conV[1].data.val.arr.p = vel;
		conV[1].value = &conV[1].data.val.arr.p;
		conV[2].value = &Particles::types;
	}
	else {
		if (setpos) {
			vpos.clear();
			vpos.reserve(Particles::particleSz);
		}
		if (setvel) {
			vvel.clear();
			vvel.reserve(Particles::particleSz);
		}
		if (settyp) {
			vtyp.clear();
			vtyp.reserve(Particles::particleSz);
		}
		int off = 0;
		if ((filter & (int)FILTER::VIS) > 0) {
			for (auto& dl : ParGraphics::drawLists) {
				for (int a = 0; a < dl.second.first; a++) {
					int i = a + dl.first;
					if (Particles::visii[i]) {
						if (setpos) vpos.push_back(Particles::poss[i]);
						if (setvel) vvel.push_back(Particles::vels[i]);
						if (settyp) vtyp.push_back(Particles::types[i]);
					}
				}
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
		else if (!!filter) {
			if (setpos) {
				vpos.resize(Particles::particleSz);
				memcpy(vpos.data(), Particles::poss, Particles::particleSz * sizeof(glm::dvec3));
			}
			if (setvel) {
				vvel.resize(Particles::particleSz);
				memcpy(vvel.data(), Particles::vels, Particles::particleSz * sizeof(glm::dvec3));
			}
			if (settyp) {
				vtyp.resize(Particles::particleSz);
				memcpy(vtyp.data(), Particles::types.data(), Particles::particleSz * sizeof(short));
			}
			parcount = Particles::particleSz;
		}
		if ((filter & (int)FILTER::CLP) > 0 && ParGraphics::clippingType != ParGraphics::CLIPPING::NONE) {
			std::vector<glm::dvec3> tpos; std::swap(tpos, vpos); vpos.reserve(parcount);
			std::vector<glm::dvec3> tvel; if (setvel) { std::swap(tvel, vvel); vvel.reserve(parcount); }
			std::vector<short> ttyp; if (settyp) { std::swap(ttyp, vtyp); vtyp.reserve(parcount); }
			for (int a = 0; a < parcount; ++a) {
				auto& pos = tpos[a];
				bool clipped = false;
				for (int c = 0; c < 6; ++c) {
					Vec4 _pos = ParGraphics::lastMV * Vec4(pos, 1);
					_pos /= _pos.w;
					if (glm::dot((Vec3)_pos, (Vec3)ParGraphics::clippingPlanes[c]) > ParGraphics::clippingPlanes[c].w) {
						clipped = true;
						break;
					}
				}
				if (clipped) continue;
				vpos.push_back(pos);
				if (setvel) vvel.push_back(tvel[a]);
				if (settyp) vtyp.push_back(ttyp[a]);
			}
			parcount = (uint)vpos.size();
		}
		poss = &vpos[0][0];
		vels = &vvel[0][0];
		typs = &vtyp[0];
		conV[0].value = &poss;
		conV[1].value = &vels;
		conV[2].value = &typs;
	}
#endif
}

void Node_Inputs::SaveIn(const std::string& path) {
	Execute();
	const auto& nm = scr.name;
	std::ofstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		for (uint i = 0; i < 3; i++)
			conV[i].Write(strm);
	}
}

void Node_Inputs::LoadIn(const std::string& path) {
	const auto& nm = scr.name;
	std::ifstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		for (uint i = 0; i < 3; i++)
			conV[i].Read(strm);
	}
	else {
		Debug::Error("Node", "cannot open input file!");
	}
}