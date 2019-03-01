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

}

void PyNode::PreExecute() {
	AnNode::PreExecute();
}

void PyNode::Execute() {
	for (uint i = 0; i < _scr->inputs.size(); ++i) {
		auto& mv = _scr->inputs[i];
		if (HasConnI(i)) {
			auto v = inputR[i].getval();
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
					szs[j] = *inputR[i].getdim(j);
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

void PyNode::Reconn() {
	AnNode::Reconn();
}

void PyNode::CatchExp(char* c) {
	
}