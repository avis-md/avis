#include "anweb.h"

//#define NO_PYLOG

PyVar::~PyVar() {
	if (value) Py_DECREF(value);
}


std::unordered_map<std::string, PyScript*> PyScript::allScrs;

PyObject* PyScript::mainModule, *PyScript::logCatcher, *PyScript::emptyString;

void PyScript::InitLog() {
#ifndef NO_PYLOG
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
#endif
}

std::string PyScript::GetLog() {
	if (!logCatcher) return "";
	PyObject *output = PyObject_GetAttrString(logCatcher, "value"); //get the stdout and stderr from our catchOutErr object
	auto u = PyUnicode_AsEncodedString(output, "UTF-8", "strict");
	char* res = PyBytes_AsString(u);
	Py_DECREF(u);
	Py_DECREF(output);
	std::string s(res);
	if (s.back() == '\n') s.pop_back();
	return s;
}

void PyScript::ClearLog() {
	if (!logCatcher) return;
	PyObject_SetAttrString(logCatcher, "value", emptyString);
}

bool PyScript::Clear() {
	AnScript::Clear();
	_invars.clear();
	_outvars.clear();
	return true;
}

std::string PyScript::Exec() {
	auto res = PyObject_CallObject(pFunc, 0);
	if (res) Py_DECREF(res);
	else {
		PyErr_Print();
		throw "\x01";
	}
	for (uint i = 0; i < outvars.size(); ++i) {
		if (_outvars[i].value) Py_DECREF(_outvars[i].value);
		_outvars[i].value = PyObject_GetAttrString(pModule, _outvars[i].name.c_str());
	}
	return "";
}

void PyScript::Set(uint i, int v) {
	if (invars.size() <= i) return;
	auto o = PyLong_FromLong(v);
	auto res = PyObject_SetAttrString(pModule, _invars[i].name.c_str(), o);
	Py_DECREF(o);
	if (res == -1) {
		Debug::Warning("PyScript::SetInt", "Failed to set value for variable \"" + _invars[i].name + "\"!");
	}
}

void PyScript::Set(uint i, double v) {
	if (invars.size() <= i) return;
	auto o = PyFloat_FromDouble(v);
	auto res = PyObject_SetAttrString(pModule, _invars[i].name.c_str(), o);
	Py_DECREF(o);
	if (res == -1) {
		Debug::Warning("PyScript::SetDouble", "Failed to set value for variable \"" + _invars[i].name + "\"!");
	}
}

void PyScript::Set(uint i, PyObject* o) {
	if (invars.size() <= i) return;
	auto res = PyObject_SetAttrString(pModule, _invars[i].name.c_str(), o);
	if (res == -1) {
		Debug::Warning("PyScript::SetObj", "Failed to set value for variable \"" + _invars[i].name + "\"!");
	}
}
