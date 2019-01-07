#include "anweb.h"
#include "anconv.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

PyNode::PyNode(PyScript* scr) : AnNode(scr) {
	if (!scr) return;
	title = scr->name + " (python)";
	inputV.resize(scr->invars.size());
	outputV.resize(scr->outvars.size());
	outputVC.resize(scr->outvars.size());
	inputVDef.resize(scr->invars.size());
	for (uint i = 0; i < scr->invars.size(); ++i) {
		inputV[i] = scr->_invars[i];
		if (scr->_invars[i].type == AN_VARTYPE::DOUBLE) inputVDef[i].d = 0;
		else inputVDef[i].i = 0;
	}
	for (uint i = 0; i < scr->outvars.size(); ++i) {
		auto& ovi = outputV[i] = scr->_outvars[i];
		conV[i].type = ovi.type;
		conV[i].typeName = ovi.typeName;
		if (ovi.type == AN_VARTYPE::LIST) {
			conV[i].dimVals.resize(ovi.dim);
			outputVC[i].dims.resize(ovi.dim);
		}
	}
	RemoveFrames();
}

void PyNode::Execute() {
	auto scr = (PyScript*)script;
	for (uint i = 0; i < script->invars.size(); ++i) {
		auto& mv = scr->_invars[i];
		if (HasConnI(i)) {
			auto& cv = inputR[i].getconv();
			switch (mv.type) {
			case AN_VARTYPE::INT:
				scr->Set(i, *((int*)cv.value));
				break;
			case AN_VARTYPE::DOUBLE:
				scr->Set(i, *((double*)cv.value));
				break;
			case AN_VARTYPE::LIST: {
				auto sz = cv.dimVals.size();
				std::vector<int> dims(sz);
				for (size_t a = 0; a < sz; a++)
					dims[a] = *cv.dimVals[a];
				scr->Set(i, AnConv::PyArr(inputR[i].first->script->outvars[inputR[i].second].second[6], (int)sz, &dims[0], *(void**)cv.value));
				break; 
			}
			default:
				OHNO("AnVar", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
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
				Debug::Error("PyNode", "Value not handled!\1");
				throw "";
				break;
			default:
				OHNO("AnVar", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
	}
	script->Exec();
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
			ar.p = AnConv::FromPy(mv.value, (int)conV[i].dimVals.size(), &conV[i].dimVals[0], n);
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
	for (int a = 0; a < conV.size(); ++a) {
		conVAll[a][f] = outputVC[a];
		conVAll[a][f].val.arr.p = &conVAll[a][f].val.arr.data[0];
	}
}

void PyNode::RemoveFrames() {
	AnNode::RemoveFrames();
	auto scr = (PyScript*)script;
	for (uint i = 0; i < scr->outvars.size(); ++i) {
		switch (scr->_outvars[i].type) {
		case AN_VARTYPE::INT:
			conV[i].value = &outputVC[i].val.i;
			break;
		case AN_VARTYPE::DOUBLE:
			conV[i].value = &outputVC[i].val.d;
			break;
		case AN_VARTYPE::LIST:
			for (int j = 0; j < outputV[i].dim; j++)
				conV[i].dimVals[j] = &outputVC[i].dims[j];
			conV[i].value = &outputVC[i].val.arr.p;
			break;
		default:
			OHNO("AnVar", "Unexpected scr_vartype " + std::to_string((int)(scr->_outvars[i].type)));
			throw "";
		}
	}
}

void PyNode::CatchExp(char* c) {
	if (!c[0]) return;
	auto ss = string_split(c, '\n');
	if (ss.size() == 1 && ss[0] == "") return;

	size_t i = 0;
	for (auto& s : ss) {
		if (s == "Traceback (most recent call last):") {
			break;
		}
		else {
			log.push_back(std::pair<byte, std::string>(0, s));
			i++;
		}
	}
	auto n = ss.size() - 1;
	log.push_back(std::pair<byte, std::string>(2, ss[n-1]));
	ErrorView::Message msg{};
	msg.name = script->name;
	msg.path = script->path;
	msg.severe = true;
	msg.msg.resize(n - i - 1);
	msg.msg[0] = ss[n-1];
	for (size_t a = i + 1; a < n - 1; ++a) {
		msg.msg[a - i] = ss[a];
	}
	auto loc = string_find(ss[n - 3], ", line ");
	msg.linenum = atoi(&ss[n - 3][loc + 7]);
	ErrorView::execMsgs.push_back(msg);
}