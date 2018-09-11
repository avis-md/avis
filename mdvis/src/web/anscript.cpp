#include "anweb.h"

//#define NO_PYLOG

void AnScript::Clear() {
	invars.clear();
	outvars.clear();
	desc = "";
	descLines = 0;
	ok = false;
}

int AnScript::StrideOf(char c) {
	switch (c) {
	case 's':
		return 2;
	case 'i':
		return 4;
	case 'd':
		return 8;
	default:
		return 0;
	}
}


std::unordered_map<string, PyScript*> PyScript::allScrs;

PyObject* PyScript::mainModule, *PyScript::logCatcher, *PyScript::emptyString;

void PyScript::InitLog() {
#ifdef NO_PYLOG
	return;
#endif
	std::string stdOutErr = "import sys\n\
class CatchOutErr:\n\
    value = ''\n\
    def write(self, txt):\n\
        self.value += txt\n\
    def clear(self):\n\
        self.value = ''\n\
catchOutErr = CatchOutErr()\n\
sys.stdout = catchOutErr\n\
sys.stderr = catchOutErr\n\
";
	mainModule = PyImport_AddModule("__main__");
	PyRun_SimpleString(stdOutErr.c_str());
	logCatcher = PyObject_GetAttrString(mainModule, "catchOutErr"); //get our catchOutErr created above
	emptyString = PyUnicode_FromString("");
}

string PyScript::GetLog() {
	if (!logCatcher) return "";
	PyObject *output = PyObject_GetAttrString(logCatcher, "value"); //get the stdout and stderr from our catchOutErr object
	auto u = PyUnicode_AsEncodedString(output, "UTF-8", "strict");
	char* res = PyBytes_AsString(u);
	Py_DECREF(u);
	Py_DECREF(output);
	string s(res);
	if (s.back() == '\n') s.pop_back();
	return s;
}

void PyScript::ClearLog() {
	if (!logCatcher) return;
	PyObject_SetAttrString(logCatcher, "value", emptyString);
}

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
		throw "\x01";
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

void PyScript::Set(uint i, PyObject* v) {
	if (invars.size() <= i) return;
	auto& vl = _invars[i].value;
	if (vl) Py_DECREF(vl);
	vl = v;
}


void CVar::Write(std::ofstream& strm) {
	_StreamWrite(&type, &strm, 1);
	switch (type) {
	case AN_VARTYPE::DOUBLE:
		_StreamWrite((double*)value, &strm, 8);
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
				strm.write(*((char**)value), totalSz * stride);
				ulong wt = (ulong)(strm.tellp() - po);
				if (wt < totalSz * stride || strm.bad()) {
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

void CVar::Read(std::ifstream& strm) {
	_Strm2Val(strm, type);
	switch (type) {
	case AN_VARTYPE::DOUBLE:
		value = &data.val.d;
		_Strm2Val(strm, data.val.d);
		break;
	case AN_VARTYPE::INT:
		value = &data.val.i;
		_Strm2Val(strm, (int32_t&)data.val.i);
		break;
	case AN_VARTYPE::LIST:
		{
			byte sz = 0;
			_Strm2Val(strm, sz);
			dimVals.resize(sz);
			data.dims.resize(sz);
			int totalSz = 1;
			for (auto a = 0; a < sz; a++) {
				dimVals[a] = &data.dims[a];
				_Strm2Val(strm, data.dims[a]);
				totalSz *= data.dims[a];
			}
			data.val.arr.data.resize(max(totalSz * stride, 1));
			data.val.arr.p = &data.val.arr.data[0];
			value = &data.val.arr.p;
			if (!!totalSz) {
				strm.read(&data.val.arr.data[0], totalSz * stride);
				if (strm.gcount() != totalSz * stride || strm.bad()) {
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
#ifdef PLATFORM_WIN
	if (useMsvc) {
		funcLoc();
	}
	else {
		auto res = wFuncLoc();
		if (res)
			throw res;
	}
#else
	funcLoc();
#endif
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
	auto res = funcLoc();
	if (res)
		throw res;
	return "";
}