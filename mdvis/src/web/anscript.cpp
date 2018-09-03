#include "anweb.h"

void AnScript::Clear() {
	invars.clear();
	outvars.clear();
	desc = "";
	descLines = 0;
	ok = false;
}

std::unordered_map<string, PyScript*> PyScript::allScrs;

void PyScript::Clear() {
	AnScript::Clear();
	_invars.clear();
	_outvars.clear();
	if (AnWeb::hasPy) {
		Py_DECREF(pArgl);
		for (auto& i : _invars) {
			if (i.value) Py_DECREF(i.value);
		}
		for (auto& r : pRets) {
			if (r) Py_DECREF(r);
		}
	}
}

string PyScript::Exec() {
	uint i = 0;
	for (; i < invars.size(); ++i) {
		auto& val = _invars[i].value;
		Py_INCREF(val);
		PyTuple_SetItem(pArgl, i, val);
	}
	auto res = PyObject_CallObject(pFunc, (!!i) ? pArgl : 0);
	if (res) Py_DECREF(res);
	else {
		PyErr_Print();
		Debug::Warning("PyScript", "Failed to execute script!");
		return "";
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

void PyScript::Set(uint i, double v) {
	if (invars.size() <= i) return;
	auto& vl = _invars[i].value;
	if (vl) Py_DECREF(vl);
	vl = PyFloat_FromDouble(v);
}

void PyScript::Set(uint i, void* v) {
	if (invars.size() <= i) return;
	_invars[i].value = (PyObject*)v;
}


void CVar::Write(std::ofstream& strm) {
	_StreamWrite(&type, &strm, 1);
	switch (type) {
	case AN_VARTYPE::DOUBLE:
		_StreamWrite((double*)value, &strm, sizeof(double));
		break;
	case AN_VARTYPE::INT:
		_StreamWrite((int32_t*)value, &strm, 4);
		break;
	case AN_VARTYPE::LIST:
		{
			int totalSz = 1;
			auto sz = dimVals.size();
			_StreamWrite(&sz, &strm, 1);
			for (uint a = 0; a < sz; a++) {
				_StreamWrite((int32_t*)dimVals[a], &strm, 4);
				totalSz *= *dimVals[a];
			}
			if (totalSz > 0) {
				auto po = strm.tellp();
				strm.write(*((char**)value), totalSz * sizeof(float));
				ulong wt = (ulong)(strm.tellp() - po);
				if (wt < totalSz * sizeof(float) || strm.bad()) {
					Debug::Error("CVar", "not enough bytes written!");
				}
			}
		}
		break;
	default:
		Debug::Warning("CVar", "write case not handled!");
		break;
	}
}

//TODO: remove dangling pointers
void CVar::Read(std::ifstream& strm) {
	_Strm2Val(strm, type);
	switch (type) {
	case AN_VARTYPE::DOUBLE:
		value = new double();
		_Strm2Val(strm, *((double**)value));
		break;
	case AN_VARTYPE::INT:
		value = new int();
		_Strm2Val(strm, *((int32_t**)value));
		break;
	case AN_VARTYPE::LIST:
		{
			byte sz = 0;
			_Strm2Val(strm, sz);
			dimVals.resize(sz);
			int totalSz = 1;
			for (auto a = 0; a < sz; a++) {
				dimVals[a] = new int();
				_Strm2Val(strm, *((int32_t*)dimVals[a]));
				totalSz *= *dimVals[a];
			}
			auto vec = new float[max(totalSz, 1)];
			value = new float*(vec);
			if (!!totalSz) {
				strm.read((char*)vec, totalSz * sizeof(float));
				if (strm.gcount() != totalSz * sizeof(float) || strm.bad()) {
					Debug::Error("CVar", "not enough bytes read!");
				}
			}
		}
		break;
	default:
		Debug::Warning("CVar", "read case not handled!");
		break;
	}
}


std::unordered_map<string, CScript*> CScript::allScrs;

void CScript::Clear() {
	AnScript::Clear();
	_invars.clear();
	_outvars.clear();
	if (AnWeb::hasC) {
		DyLib::ForceUnload(lib, libpath);
		delete(lib);
	}
}

string CScript::Exec() {
	funcLoc();
	return "";
}

std::unordered_map<string, FScript*> FScript::allScrs;

void FScript::Clear() {
	AnScript::Clear();
	_invars.clear();
	_outvars.clear();
	if (AnWeb::hasFt) {
		DyLib::ForceUnload(lib, libpath);
		delete(lib);
	}
}

string FScript::Exec() {
	funcLoc();
	return "";
}