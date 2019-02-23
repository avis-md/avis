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
	return res;
}

#define _instance ((PyObject*)instance)

PyScript_I::~PyScript_I() {
	for (uint i = 0; i < outputs.size(); ++i) {
		if (outputs[i]) Py_DECREF(outputs[i]);
	}
	Py_DECREF(_instance);
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
