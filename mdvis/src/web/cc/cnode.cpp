#include "web/anweb.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

#define _scr ((CScript*)scr->parent)

CNode::CNode(pCScript_I scr) : AnNode(scr) {
	if (!scr) return;
	title = _scr->name + " (c++)";
	const auto isz = _scr->inputs.size();
	const auto osz = _scr->outputs.size();
	inputV.resize(isz);
	outputV.resize(osz);
	inputVDef.resize(isz);
	for (size_t i = 0; i < isz; ++i) {
		inputV[i] = script->Resolve(_scr->inputs[i].offset);
		if (_scr->inputs[i].type == AN_VARTYPE::DOUBLE) inputVDef[i].d = 0;
		else inputVDef[i].i = 0;
	}
	for (uint i = 0; i < _scr->outputs.size(); ++i) {
		outputV[i] = script->Resolve(_scr->outputs[i].offset);
		conV[i] = _scr->_outputs[i];
	}
}

#undef _scr
#define _scr ((CScript*)script->parent)

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
			case AN_VARTYPE::LIST:
				script->SetInput(i, *(uintptr_t*)v);
				/*
				for (uint j = 0; j < mv.dimVals.size(); ++j) {
					auto loc = mv.dimVals[j];
					if (loc)
						*loc = *cv.dimVals[j];
				}
				*/
				break;
			default:
				OHNO("CNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
		else {
			switch (mv.type) {
			case AN_VARTYPE::INT:
				script->SetInput(i, inputVDef[i].i);
				break;
			case AN_VARTYPE::DOUBLE:
				script->SetInput(i, inputVDef[i].d);
				break;
			case AN_VARTYPE::LIST:
				throw("Input variable not set!\1");
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