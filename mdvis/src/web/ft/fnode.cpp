#include "web/anweb.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

#define _scr ((FScript*)script->parent)

FNode::FNode(pFScript_I scr) : AnNode(scr) {
	if (!scr) return;
	title = _scr->name + " (fortran)";
	const auto isz = _scr->inputs.size();
	const auto osz = _scr->outputs.size();
	for (uint i = 0; i < osz; ++i) {
	}
	for (uint i = 0; i < osz; ++i) {
		auto& cv = conV[i];
		if (_scr->outputs[i].type == AN_VARTYPE::LIST) {
			cv.offset = i;
			auto d = _scr->outputs[i].dim;
			cv.szOffsets.resize(d);
			for (int j = 0; j < d; j++) {
				cv.szOffsets[j].offset = (i << 16) | j;
			}
		}
		else {
			cv = _scr->_outputs[i];
		}
	}
}

void FNode::Update() {}

void FNode::PreExecute() {
	AnNode::PreExecute();
}

void FNode::Execute() {
	for (uint i = 0; i < _scr->inputs.size(); ++i) {
		_scr->pre = i;
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
			case AN_VARTYPE::LIST:
			{
				auto& vr = inputR[i].getvar();
				std::vector<int> szs(vr.dim);
				for (uint j = 0; j < vr.dim; ++j) {
					szs[j] = inputR[i].getdim(j);
				}
				script->SetInput(i, *(void**)v, mv.name[6], szs);
				break;
			}
			default:
				OHNO("CNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
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
				script->SetInput(i, (void*)1, mv.name[6], szs);
				break;
			}
			default:
				OHNO("CNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
	}
	_scr->pre = -1;
	_scr->post = -1;
	
	script->Execute();

	((FScript_I*)script.get())->GetOutputArrs();
}

void FNode::WriteFrame(int f) {

}

void FNode::RemoveFrames() {
	
}

void FNode::Reconn() {}

void FNode::CatchExp(char* c) {
	std::string s = c;
	if (s.back() == -1) {
		AnNode::CatchExp(c);
		return;
	}
	ErrorView::Message msg{};
	msg.name = _scr->name;
	msg.path = _scr->path;
	msg.severe = true;
	if (string_find(s, "At line ") == 0) {
		if ((msg.linenum = atoi(c + 8)) > 0) {
			auto lc = strchr(c, '\n');
			log.push_back(std::pair<byte, std::string>(2, lc + 1));
			std::string s(c + 9, (size_t)lc - (size_t)c - 20); //_temp__.f90\n
			msg.msg.resize(1, lc + 1);
			msg.msg.push_back("Fortran runtime error caught by handler");
			ErrorView::execMsgs.push_back(msg);
			return;
		}
	}
	else if (s.back() == 1) {
		s.pop_back();
		log.push_back(std::pair<byte, std::string>(2, s));
		msg.msg.resize(1, s);
		auto scr = (FScript_I*)script.get();
		if (_scr->pre > -1) {
			msg.msg.push_back("While handling input variable " + _scr->inputs[_scr->pre].name);
		}
		else if (_scr->post > -1) {
			msg.msg.push_back("While handling output variable " + _scr->outputs[_scr->post].name);
		}
		ErrorView::execMsgs.push_back(msg);
		return;
	}
	AnNode::CatchExp(c);
}