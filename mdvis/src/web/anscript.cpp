#include "anweb.h"

void DmScript::Set(uint i, int v) {
	
}

void DmScript::Set(uint i, float v) {
	
}

void DmScript::Set(uint i, void* v) {
	
}

void* DmScript::Get(uint i) {
	return nullptr;
}


std::unordered_map<string, PyScript*> PyScript::allScrs;

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
	case AN_VARTYPE::INT:
		return new int(_PyLong_AsInt(pRets[i]));
		break;
	case AN_VARTYPE::FLOAT:
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


void CVar::Write(std::ofstream& strm) {
	_StreamWrite(&type, &strm, 1);
	switch (type) {
	case AN_VARTYPE::FLOAT:
		_StreamWrite((float*)value, &strm, sizeof(float));
		break;
	case AN_VARTYPE::INT:
		_StreamWrite((int32_t*)value, &strm, sizeof(int32_t));
		break;
	case AN_VARTYPE::LIST:
		int32_t sz = (int32_t)dimVals.size();
		_StreamWrite(&sz, &strm, sizeof(int32_t));
		int totalSz = 0;
		for (auto a = 0; a < sz; a++) {
			_StreamWrite((int32_t*)dimVals[a], &strm, sizeof(int32_t));
			totalSz *= *dimVals[a];
		}
		_StreamWrite(*((float**)value), &strm, totalSz * sizeof(float));
		break;
	}
}

void CVar::Read(std::ifstream& strm) {
	_Strm2Val(strm, type);
	switch (type) {
	case AN_VARTYPE::FLOAT:
		value = new float();
		_Strm2Val(strm, *((float**)value));
		break;
	case AN_VARTYPE::INT:
		value = new int();
		_Strm2Val(strm, *((int32_t**)value));
		break;
	case AN_VARTYPE::LIST:
		int32_t sz = 0;
		_Strm2Val(strm, *((int32_t**)sz));
		dimVals.resize(sz);
		int totalSz = 0;
		for (auto a = 0; a < sz; a++) {
			dimVals[a] = new int();
			_Strm2Val(strm, *dimVals[a]);
			totalSz *= *dimVals[a];
		}
		value = new uintptr_t();
		auto loc = *((float**)value) = new float[totalSz];
		strm.read((char*)loc, totalSz * sizeof(float));
		break;
	}
}


std::unordered_map<string, CScript*> CScript::allScrs;

string CScript::Exec() {
	funcLoc();
	return "";
}

void CScript::Set(uint i, int v) {
	*((int*)_invars[i].value) = v;
}

void CScript::Set(uint i, float v) {
	*((float*)_invars[i].value) = v;
}

void CScript::Set(uint i, void* v) {

}

void* CScript::Get(uint i) {

	return nullptr;
}