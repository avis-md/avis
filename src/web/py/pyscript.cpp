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
#include "web/py/pyreader.h"
#include "pyarr.h"

#define NO_PYLOG

std::unordered_map<std::string, std::weak_ptr<PyScript>> PyScript::allScrs;

PyScript::PyScript() : AnScript(TYPE::PYTHON), scrpath(""), lib(nullptr), spawner(nullptr), funcNm("") {}

void PyScript::Clear() {

}

pAnScript_I PyScript::CreateInstance() {
	auto res = std::make_shared<PyScript_I>();
	res->Init(this);
	if (!isSingleton) {
		auto i = PyObject_CallObject(spawner, 0);
		res->instance = i;
		res->dict = PyObject_GetAttrString(i, "__dict__");
	}
	else {
		res->instance = (void*)1;
	}
	auto osz = outputs.size();
	res->outputVs.resize(osz);
	for (size_t a = 0; a < outputs.size(); a++) {
		res->outputVs[a].dims.resize(outputs[a].dim);
	}
	instances.push_back(res.get());
	return res;
}

void PyScript::RegInstances() {
	for (auto i : instances) {
		if (!isSingleton) {
			auto o = PyObject_CallObject(spawner, 0);
			i->instance = o;
			i->dict = PyObject_GetAttrString(o, "__dict__");
		}
		else {
			i->instance = (void*)1;
			i->dict = nullptr;
		}
	}
}

void PyScript::UnregInstances() {
	for (auto i : instances) {
		if (i->dict) {
			Py_DECREF(i->instance);
		}
	}
}

#define _instance ((PyObject*)instance)
#define _parent ((PyScript*)parent)

PyScript_I::~PyScript_I() {
	if (!parent->isSingleton) {
		Py_DECREF(_instance);
		_parent->instances.erase(std::find(_parent->instances.begin(), _parent->instances.end(), this));
	}
}

void* PyScript_I::Resolve(uintptr_t i) {
	return outputVs[i].val.pval;
}

int* PyScript_I::GetDimValue(const CVar::szItem& i) {
	return &outputVs[i.offset >> 16].dims[i.offset & 0xffff];
}

void PyScript_I::SetInput(int i, short v) {
	auto o = PyLong_FromLong(v);
	Set(i, o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, int v) {
	auto o = PyLong_FromLong(v);
	Set(i, o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, double v) {
	auto o = PyFloat_FromDouble(v);
	Set(i, o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, void* val, char tp, std::vector<int> szs) {
	auto o = PyArr::ToPy(val, (int)szs.size(), tp, szs.data());
	Set(i, o);
	Py_DECREF(o);
}

void PyScript_I::Set(int i, PyObject* o) {
	if (parent->isSingleton) {
		PyObject_SetAttrString(_parent->lib, _parent->_inputs[i].name.c_str(), o);
	}
	else {
		PyDict_SetItemString(dict, _parent->_inputs[i].name.c_str(), o);
	}
}

void PyScript_I::GetOutput(int i, int* val) {
	;
}

void PyScript_I::Execute() {
	auto p = AnWeb::nodesPath + parent->path;
	auto cpath = p.substr(0, p.find_last_of('/') + 1) + "__pycache__/" + parent->name;
	PyObject_SetAttrString(PyReader::mainModule, "__cpath_prefix__", PyBytes_FromString(cpath.c_str()));
	PyObject_SetAttrString(PyReader::mainModule, "__plt_figcount__", PyLong_FromLong(0));
	auto res = parent->isSingleton ?
		PyObject_CallObject(_parent->func, nullptr) :
		PyObject_CallMethod(_instance, _parent->funcNm.c_str(), 0);
	if (res) Py_DECREF(res);
	else {
		PyErr_Print();
		throw "\x01";
	}

	figCount = -PyLong_AsLong(PyObject_GetAttrString(PyReader::mainModule, "__plt_figcount__"));
}

float PyScript_I::GetProgress() {
	return 0.1;
}

void PyScript_I::GetOutputVs() {
	for (uint i = 0; i < outputVs.size(); ++i) {
		auto& v = outputVs[i];
		auto& nm = _parent->_outputs[i].name;
		auto mv = PyDict_GetItemString(dict, nm.c_str());
		auto& ot = _parent->outputs[i];
		switch (ot.type) {
		case AN_VARTYPE::INT:
			v.val.val.i = (int)PyLong_AsLong(mv);
			v.val.pval = &v.val.val.i;
			break;
		case AN_VARTYPE::DOUBLE:
			v.val.val.d = PyFloat_AsDouble(mv);
			v.val.pval = &v.val.val.d;
			break;
		case AN_VARTYPE::LIST: {
			int n;
			void* p;
			if (!(p = PyArr::FromPy(mv, ot.dim, ot.stride, v.dims.data(), n)))
				throw "";
			n *= ot.stride;
			v.val.arr.resize(n);
			memcpy((v.val.val.p = v.val.arr.data()), p, n);
			v.val.pval = &v.val.val.p;
			break;
		}
		default:
			OHNO("AnVar", "Unexpected scr_vartype " + std::to_string((int)ot.type));
			throw "";
		}
	}
}