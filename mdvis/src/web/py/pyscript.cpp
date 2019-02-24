#include "web/anweb.h"
#include "pyarr.h"

#define NO_PYLOG

std::unordered_map<std::string, std::weak_ptr<PyScript>> PyScript::allScrs;

void PyScript::Clear() {

}

pAnScript_I PyScript::CreateInstance() {
	auto res = std::make_shared<PyScript_I>();
	res->Init(this);
	auto i = PyObject_CallObject(spawner, 0);
	res->instance = i;
	res->func = PyObject_GetAttrString(i, funcNm.c_str());
	auto osz = outputs.size();
	res->outputs.resize(osz);
	res->outputVs.resize(osz);
	return res;
}

#define _instance ((PyObject*)instance)

PyScript_I::~PyScript_I() {
	for (uint i = 0; i < outputs.size(); ++i) {
		if (outputs[i]) Py_DECREF(outputs[i]);
	}
	Py_DECREF(_instance);
}

void* PyScript_I::Resolve(uintptr_t i) {
	return outputVs[i].val.pval;
}

int* PyScript_I::GetDimValue(const CVar::szItem& i) {
	return i.useOffset ?
		&outputVs[i.offset >> 16].dims[i.offset & 0xffff]
		: (int*)&i.size;
}

void PyScript_I::Execute() {
	auto res = PyObject_CallObject(func, 0);
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

void PyScript_I::SetInput(int i, short v) {
	auto o = PyLong_FromLong(v);
	auto res = PyObject_SetAttrString(_instance, parent->inputs[i].name.c_str(), o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, int v) {
	auto o = PyLong_FromLong(v);
	auto res = PyObject_SetAttrString(_instance, parent->inputs[i].name.c_str(), o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, double v) {
	auto o = PyFloat_FromDouble(v);
	auto res = PyObject_SetAttrString(_instance, parent->inputs[i].name.c_str(), o);
	Py_DECREF(o);
}

void PyScript_I::SetInput(int i, void* val, char tp, std::vector<int> szs) {
	auto o = PyObject_GetAttrString(_instance, parent->inputs[i].name.c_str());
	PyArr::ToPy(val, o, (int)szs.size(), szs.data());
}

#define _parent ((PyScript*)parent)

void PyScript_I::GetOutputVs() {
	for (uint i = 0; i < outputs.size(); ++i) {
		auto& mv = outputs[i];
		auto& v = outputVs[i];
		if (mv) Py_DECREF(mv);
		mv = PyObject_GetAttrString(_instance, _parent->_outputs[i].name.c_str());
		Py_INCREF(mv);
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
			if (!(v.val.val.p = PyArr::FromPy(mv, ot.dim, ot.stride, v.dims.data(), n)))
				throw "";
			//n *= ot.stride;
			//ar.data.resize(n);
			//memcpy(&ar.data[0], ar.p, n);
			//ar.p = &ar.data[0];
			v.val.pval = &v.val.val.p;
			break;
		}
		default:
			OHNO("AnVar", "Unexpected scr_vartype " + std::to_string((int)ot.type));
			throw "";
		}
	}
}