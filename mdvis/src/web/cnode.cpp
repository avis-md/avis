#include "anweb.h"
#include "anconv.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

CNode::CNode(CScript* scr) : AnNode(scr) {
	if (!scr) return;
	title += " (c++)";
	inputV.resize(scr->invars.size());
	outputV.resize(scr->outvars.size());
	inputVDef.resize(scr->invars.size());
	for (uint i = 0; i < scr->invars.size(); ++i) {
		inputV[i] = scr->_invars[i].value;
		if (scr->_invars[i].type == AN_VARTYPE::DOUBLE) inputVDef[i].d = 0;
		else inputVDef[i].i = 0;
	}
	for (uint i = 0; i < scr->outvars.size(); ++i) {
		outputV[i] = scr->_outvars[i].value;
		conV[i] = scr->_outvars[i];
	}
}

void CNode::Execute() {
	auto scr = (CScript*)script;
	for (uint i = 0; i < scr->invars.size(); ++i) {
		auto& mv = scr->_invars[i];
		if (HasConnI(i)) {
			auto& cv = inputR[i].first->conV[inputR[i].second];
			switch (mv.type) {
			case AN_VARTYPE::INT:
				scr->Set(i, *(int*)cv.value);
				break;
			case AN_VARTYPE::DOUBLE:
				scr->Set(i, *(double*)cv.value);
				break;
			case AN_VARTYPE::LIST:
				scr->Set(i, *(uintptr_t*)cv.value);
				
				for (uint j = 0; j < mv.dimVals.size(); ++j) {
					auto loc = mv.dimVals[j];
					if (loc)
						*loc = *cv.dimVals[j];
				}
				break;
			default:
				OHNO("CNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
		else {
			switch (mv.type) {
			case AN_VARTYPE::INT:
				scr->Set(i, inputVDef[i].i);
				break;
			case AN_VARTYPE::DOUBLE:
				scr->Set(i, inputVDef[i].d);
				break;
			case AN_VARTYPE::LIST:
				throw("Input variable not set!\1");
			default:
				OHNO("CNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
	}

	script->Exec();
}

void CNode::WriteFrame(int f) {
	auto scr = (CScript*)script;
	for (int a = 0; a < conV.size(); ++a) {
		auto& c = conV[a];
		auto& ca = conVAll[a][f];
		switch (c.type) {
		case AN_VARTYPE::INT:
			ca.val.i = *(int*)scr->_outvars[a].value;
			break;
		case AN_VARTYPE::DOUBLE:
			ca.val.d = *(double*)scr->_outvars[a].value;
			break;
		case AN_VARTYPE::LIST: {
			int n = 1;
			auto ds = conV[a].dimVals.size();
			ca.dims.resize(ds);
			for (size_t d = 0; d < ds; ++d) {
				n *= ca.dims[d] = *conV[a].dimVals[d];
			}
			n *= scr->_outvars[a].stride;
			ca.val.arr.data.resize(n);
			memcpy(&ca.val.arr.data[0], *(char**)scr->_outvars[a].value, n);
			ca.val.arr.p = &ca.val.arr.data[0];
			break;
		}
		}
	}
}

void CNode::RemoveFrames() {
	AnNode::RemoveFrames();
	auto scr = (CScript*)script;
	for (uint i = 0; i < scr->outvars.size(); ++i) {
		conV[i] = scr->_outvars[i];
	}
}

void CNode::Reconn() {
	AnNode::Reconn();
}

void CNode::CatchExp(char* c) {
	ErrorView::Message msg{};
	msg.name = script->name;
	msg.path = script->path;
	msg.severe = true;
	msg.msg.resize(1, c);
	log.push_back(std::pair<byte, std::string>(2, c));
	ErrorView::execMsgs.push_back(msg);
}