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

#include "web/anweb.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

#define _scr ((PyScript*)script->parent)

PyNode::PyNode(pPyScript_I script) : AnNode(script) {
	if (!script) return;
	title = _scr->name + " (python)";
	const auto osz = _scr->outputs.size();
	for (uint i = 0; i < osz; ++i) {
		auto& cv = conV[i];
		cv.offset = i;
		auto d = _scr->outputs[i].dim;
		cv.szOffsets.resize(d);
		for (int j = 0; j < d; j++) {
			cv.szOffsets[j].offset = (i << 16) | j;
		}
	}
}

void PyNode::Update() {
	auto scr = (PyScript_I*)script.get();
	if (scr->figCount < 0) {
		scr->figCount *= -1;
		scr->figures.resize(scr->figCount);
		showFigs.resize(scr->figCount, 0);
		auto p = AnWeb::nodesPath + _scr->path;
		auto cpath = p.substr(0, p.find_last_of('/') + 1) + "__pycache__/" + _scr->name;
		for (long i = 0; i < scr->figCount; i++) {
			scr->figures[i] = Texture(cpath + "_figure_" + std::to_string(i+1) + ".png");
		}
	}
}

void PyNode::PreExecute() {
	AnNode::PreExecute();
}

void PyNode::Execute() {
	for (uint i = 0; i < _scr->inputs.size(); ++i) {
		auto& mv = _scr->inputs[i];
		if (HasConnI(i)) {
			auto v = inputR[i].getval(ANVAR_ORDER::C);
			switch (mv.type) {
			case AN_VARTYPE::INT:
				script->SetInput(i, *(int*)v);
				break;
			case AN_VARTYPE::DOUBLE:
				script->SetInput(i, *(double*)v);
				break;
			case AN_VARTYPE::LIST: {
				auto& vr = inputR[i].getvar();
				std::vector<int> szs(vr.dim);
				for (uint j = 0; j < vr.dim; ++j) {
					szs[j] = inputR[i].getdim(j);
				}
				script->SetInput(i, *(void**)v, mv.typeName[6], szs);
				break;
			}
			default:
				OHNO("AnVar", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
		else {
			auto& dv = script->defVals[i];
			switch (mv.type) {
			case AN_VARTYPE::INT:
				script->SetInput(i, dv.i);
				break;
			case AN_VARTYPE::DOUBLE:
				script->SetInput(i, dv.d);
				break;
			case AN_VARTYPE::LIST: {
				std::vector<int> szs(_scr->inputs[i].dim, 0);
				script->SetInput(i, nullptr, mv.typeName[6], szs);
				break;
			}
			default:
				OHNO("PyNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
	}
	script->Execute();
	((PyScript_I*)script.get())->GetOutputVs();
}

void PyNode::WriteFrame(int f) {
	AnNode::WriteFrame(f);
}

void PyNode::RemoveFrames() {
	AnNode::RemoveFrames();
}

void PyNode::DrawFooter(float& off) {
	int i = 0;
	for (auto& fig : ((PyScript_I*)script.get())->figures) {
		auto& sf = showFigs[i];
		if (Engine::Button(pos.x + 2, off, 16, 16, (!!sf) ? Icons::expand : Icons::collapse, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE)
			sf = !sf;
		UI::Label(pos.x + 20, off, 12, "Figure " + std::to_string(++i), white());
		off += 17;
		if (!!sf) {
			auto h = ((width - 4) * fig.height) / fig.width;
			UI::Texture(pos.x + 2, off, width - 4, h, fig, white());
			off += h + 2;
		}
	}
}

void PyNode::Reconn() {
	AnNode::Reconn();
}

void PyNode::CatchExp(char* c) {
	
}