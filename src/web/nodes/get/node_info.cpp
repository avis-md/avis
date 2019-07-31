// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#include "node_info.h"
#include "web/anweb.h"
#include "md/particles.h"

INODE_DEF(__("System Info"), Info, GET)

Node_Info::Node_Info() : INODE_INITF(AN_FLAG_NOSAVECONV) {
	INODE_TITLE(NODE_COL_IO)
	INODE_SINIT(
		scr->AddOutput(_("atom count"), AN_VARTYPE::INT);
		scr->AddOutput(_("frame count"), AN_VARTYPE::INT);
		scr->AddOutput(_("current frame"), AN_VARTYPE::INT);
		scr->AddOutput(_("bounding box"), AN_VARTYPE::DOUBLE, 1);
	);
	IAddConV(&Particles::particleSz);
	IAddConV(&Particles::anim.frameCount);
	IAddConV(&AnWeb::realExecFrame);
	IAddConV(nullptr, { nullptr }, { 6 });
	/*
	const CVar cv("", AN_VARTYPE::INT);
	AddOutput(cv);
	scr.AddOutput(_("atom count"), "int");
	AddOutput(cv);
	scr.AddOutput(_("frame count"), "int");
	AddOutput(cv);
	scr.AddOutput(_("current frame"), "int");
	AddOutput(CVar("bounding box", 'd', 1, { nullptr }, { 6 }));
	scr.AddOutput(conV.back());
	conV[0].value = &Particles::particleSz;
	conV[1].value = &Particles::anim.frameCount;
	conV[2].value = &AnWeb::realExecFrame;
	*/
}

void Node_Info::Execute() {
	static double* bbx = nullptr;
	if (!Particles::anim.bboxs.size()) bbx = &Particles::boundingBox[0];
	else if (!AnWeb::execFrame) bbx = &Particles::anim.bboxs[Particles::anim.currentFrame][0];
	else bbx = &Particles::anim.bboxs[AnWeb::execFrame-1][0];
	auto& ov = ((DmScript_I*)script.get())->outputVs;
	ov[3].val.p = bbx;
}