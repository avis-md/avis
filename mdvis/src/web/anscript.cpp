#include "anweb.h"

string PyScript::Exec() {
	for (uint i = 0; i < invars.size(); ++i) {
		auto& val = _invars[i].value;
		Py_INCREF(val);
		PyTuple_SetItem(pArgs, i, val);
	}
	auto res = PyObject_CallObject(pFunc, pArgs);
	if (res) Py_DECREF(res);
	else {
		PyErr_Print();
		__debugbreak();
	}
	for (uint i = 0; i < outvars.size(); i++) {
		Py_DECREF(pRets[i]);
		pRets[i] = PyObject_GetAttrString(pModule, _outvars[i].name.c_str());
	}
	return "";
}

void PyScript::Set(uint i, int v) {
	if (invars.size() <= i) return;
	auto& vl = _invars[i].value;
	if (vl) Py_DECREF(vl);
	vl = PyLong_FromLong(v);
}

void PyScript::Set(uint i, float v) {
	if (invars.size() <= i) return;
	auto& vl = _invars[i].value;
	if (vl) Py_DECREF(vl);
	vl = PyFloat_FromDouble(v);
}

void PyScript::Set(uint i, void* v) {
	if (invars.size() <= i) return;
	_invars[i].value = (PyObject*)v;
}

void* PyScript::Get(uint i) {
	if (outvars.size() <= i) return nullptr;
	switch (_outvars[i].type) {
	case PY_VARTYPE::INT:
		return new int(_PyLong_AsInt(pRets[i]));
		break;
	case PY_VARTYPE::FLOAT:
		return new float((float)PyFloat_AsDouble(pRets[i]));
		break;
	default:
		Debug::Error("PyScript", "Get for this type not implemented!");
		return nullptr;
		break;
	}
	if (_outvars[i].typeName == "list(float)") {
		auto sz = PyList_Size(pRets[i]);
		std::vector<float>* v = new std::vector<float>(sz);
		for (Py_ssize_t a = 0; a < sz; a++) {
			auto obj = PyList_GetItem(pRets[i], a);
			(*v)[a] = (float)PyFloat_AsDouble(obj);
		}
		return v;
	}
	Debug::Warning("PyScript", "Cannot convert type \"" + _outvars[i].typeName + "\" to C++ type!");
	return nullptr;
}