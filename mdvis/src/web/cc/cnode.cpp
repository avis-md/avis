#include "web/anweb.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

#define _scr ((CScript*)script->parent)

CNode::CNode(pCScript_I script) : AnNode(script) {
	if (!script) return;
	title = _scr->name + " (c++)";
	const auto isz = _scr->inputs.size();
	const auto osz = _scr->outputs.size();
	for (uint i = 0; i < osz; ++i) {
		conV[i] = _scr->_outputs[i];
	}
}

void CNode::Update() {
	//if (executing) {
		volatile int cnt = *_scr->stdioCnt;
		while (_scr->stdioI < cnt) {
			std::lock_guard<std::mutex> lock(*_scr->stdioLock);
			auto pptr = *_scr->stdioPtr;
			auto ptr = pptr[_scr->stdioI * 2];
			for (auto& n : AnWeb::nodes) {
//				if (((CScript*)n->script->parent) == scr) {
					if (ptr == n->script->instance) {
						auto msg = (char*)pptr[_scr->stdioI * 2 + 1];
						std::cout << "Node " + std::to_string(n->id) + " says: "
							+ std::string(msg) << std::endl;
						_scr->stdioI++;
					}
//				}
			}
		}
	//}
}

void CNode::PreExecute() {
	AnNode::PreExecute();
	_scr->stdioClr();
	_scr->stdioI = 0;
}

void CNode::Execute() {
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
				script->SetInput(i, *(void**)v, mv.name[6], szs);
			}
				break;
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

	script->Execute();
}

void CNode::WriteFrame(int f) {
	AnNode::WriteFrame(f);
	return;
	/*
	auto scr = (CScript*)script;
	for (int a = 0; a < conV.size(); ++a) {
		auto& c = conV[a];
		auto& ca = conVAll[a][f];
		switch (c.type) {
		case AN_VARTYPE::INT:
			ca.val.i = *(int*)_scr->_outvars[a].value;
			break;
		case AN_VARTYPE::DOUBLE:
			ca.val.d = *(double*)_scr->_outvars[a].value;
			break;
		case AN_VARTYPE::LIST: {
			int n = 1;
			auto ds = conV[a].dimVals.size();
			ca.dims.resize(ds);
			for (size_t d = 0; d < ds; ++d) {
				n *= ca.dims[d] = *conV[a].dimVals[d];
			}
			n *= _scr->_outvars[a].stride;
			ca.val.arr.data.resize(n);
			memcpy(&ca.val.arr.data[0], *(char**)_scr->_outvars[a].value, n);
			ca.val.arr.p = &ca.val.arr.data[0];
			break;
		}
		}
	}
	*/
}

void CNode::RemoveFrames() {
	AnNode::RemoveFrames();
	for (uint i = 0; i < _scr->_outputs.size(); ++i) {
		conV[i] = _scr->_outputs[i];
	}
}

void CNode::Reconn() {
	AnNode::Reconn();
}

void CNode::CatchExp(char* c) {
	ErrorView::Message msg{};
	msg.name = _scr->name;
	msg.path = _scr->path;
	msg.severe = true;
	msg.msg.resize(1, c);
	log.push_back(std::pair<byte, std::string>(2, c));
	ErrorView::execMsgs.push_back(msg);
}