#include "web/anweb.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

#define _scr ((CScript*)script->parent)

PyNode::PyNode(pPyScript_I script) : AnNode(script) {
	if (!script) return;
	title = _scr->name + " (python)";
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
				script->SetInput(i, *(void**)v, mv.name[6], szs);
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
			case AN_VARTYPE::LIST:
				throw("Input variable not set!\1");
			default:
				OHNO("PyNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
	}
	script->Execute();
	
	for (uint i = 0; i < script->outvars.size(); ++i) {
		auto& mv = outputV[i];
		mv.value = scr->_outvars[i].value;
		Py_INCREF(mv.value);
		switch (mv.type) {
		case AN_VARTYPE::DOUBLE:
			outputVC[i].val.d = PyFloat_AsDouble(mv.value);
			break;
		case AN_VARTYPE::INT:
			outputVC[i].val.i = (int)PyLong_AsLong(mv.value);
			break;
		case AN_VARTYPE::LIST:
		{
			auto& ar = outputVC[i].val.arr;
			int n;
			ar.p = AnConv::FromPy(mv.value, (int)conV[i].dimVals.size(), scr->_outvars[i].stride, &conV[i].dimVals[0], n);
			if (!ar.p) throw "";
			n *= scr->_outvars[i].stride;
			ar.data.resize(n);
			memcpy(&ar.data[0], ar.p, n);
			ar.p = &ar.data[0];
			break;
		}
		default:
			OHNO("AnVar", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
			throw "";
		}
	}
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