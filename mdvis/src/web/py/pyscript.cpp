#include "web/anweb.h"
#include "pyarr.h"

#define NO_PYLOG

std::unordered_map<std::string, std::weak_ptr<PyScript>> PyScript::allScrs;

PyScript::PyScript() : AnScript(TYPE::PYTHON), scrpath(""), lib(nullptr), spawner(nullptr), funcNm("") {}

void PyScript::Clear() {

}

pAnScript_I PyScript::CreateInstance() {
	auto res = std::make_shared<PyScript_I>();
	res->Init(this);
	auto i = PyObject_CallObject(spawner, 0);
	res->instance = i;
	res->dict = PyObject_GetAttrString(i, "__dict__");
	res->func = PyObject_GetAttrString(i, funcNm.c_str());
	auto osz = outputs.size();
	res->outputs.resize(osz);
	res->outputVs.resize(osz);
	for (size_t a = 0; a < outputs.size(); a++) {
		res->outputVs[a].dims.resize(outputs[a].dim);
	}
	return res;
}

#define _instance ((PyObject*)instance)
#define _parent ((PyScript*)parent)

PyScript_I::~PyScript_I() {
	Py_DECREF(_instance);
}

void* PyScript_I::Resolve(uintptr_t i) {
	return outputVs[i].val.pval;
}

int* PyScript_I::GetDimValue(const CVar::szItem& i) {
	return &outputVs[i.offset >> 16].dims[i.offset & 0xffff];
}

void PyScript_I::SetInput(int i, short v) {
	auto o = PyLong_FromLong(v);
	PyDict_SetItemString(dict, _parent->_inputs[i].name.c_str(), o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, int v) {
	auto o = PyLong_FromLong(v);
	PyDict_SetItemString(dict, _parent->_inputs[i].name.c_str(), o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, double v) {
	auto o = PyFloat_FromDouble(v);
	PyDict_SetItemString(dict, _parent->_inputs[i].name.c_str(), o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, void* val, char tp, std::vector<int> szs) {
	auto o = PyArr::ToPy(val, (int)szs.size(), tp, szs.data());
	PyDict_SetItemString(dict, _parent->_inputs[i].name.c_str(), o);
	Py_DECREF(o);
}

void PyScript_I::GetOutput(int i, int* val) {
	;
}

void PyScript_I::Execute() {
	auto res = PyObject_CallMethod(_instance, _parent->funcNm.c_str(), 0);
	if (res) Py_DECREF(res);
	else {
		PyErr_Print();
		throw "\x01";
	}
	for (uint i = 0; i < outputs.size(); ++i) {
		if (outputs[i]) Py_DECREF(outputs[i]);
		outputs[i] = PyObject_GetAttrString(_instance, parent->outputs[i].name.c_str());
	}
}

float PyScript_I::GetProgress() {
	return 0.1;
}

void PyScript_I::GetOutputVs() {
	for (uint i = 0; i < outputs.size(); ++i) {
		auto& mv = outputs[i];
		auto& v = outputVs[i];
		//mv = PyObject_GetAttrString(_instance, _parent->_outputs[i].name.c_str());
		auto& nm = _parent->_outputs[i].name;
		mv = PyDict_GetItemString(dict, nm.c_str());
		continue;
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