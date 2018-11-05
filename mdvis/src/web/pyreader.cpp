#include "anweb.h"
#include "anscript.h"
#include "anconv.h"
#include "vis/system.h"
#ifndef PLATFORM_WIN
#include <dlfcn.h>
#endif

bool PyReader::initd = false;

void PyReader::Init() {
	initd = true;
#ifndef PLATFORM_WIN
#ifdef PLATFORM_LNX
	//auto lib = dlopen("libpython3.6m.so", RTLD_LAZY | RTLD_GLOBAL);
#else
	//auto lib = dlopen("/Library/Frameworks/Python.framework/Versions/3.6/Python", RTLD_NOW | RTLD_GLOBAL);
#endif
	if (dlsym(RTLD_DEFAULT, "Py_Initialize")) {
#endif
	static std::string pyenv = VisSystem::envs["PYENV"];
#ifdef PLATFORM_WIN
	size_t psz;
	getenv_s(&psz, 0, 0, "PATH");
	char* pbuf = new char[psz];
	getenv_s(&psz, pbuf, psz, "PATH");
	_putenv_s("PATH", &(pyenv + "\\;" + std::string(pbuf))[0]);
	delete[](pbuf);

	static std::wstring pyenvw = IO::_tow(pyenv);
	Py_SetPythonHome(&pyenvw[0]);
	static std::wstring pynmw = IO::_tow(pyenv + "\\python");
	Py_SetProgramName(&pynmw[0]);
#else
	//Py_SetPythonHome(&pyenv[0]);
	//Py_SetProgramName("python3");
#endif
	try {
		Py_Initialize();
		PyObject *sys_path = PySys_GetObject("path");
		auto path = IO::path + "nodes/";
		auto ps = PyUnicode_FromString(path.c_str());
		PyList_Insert(sys_path, 0, ps);
		Py_DecRef(ps);

		if (!!AnConv::Init())
			return;

		AnWeb::hasPy = true;
		PyScript::InitLog();
	} catch (char*) {
		std::string env = "";
#ifdef PLATFORM_WIN
		char* buf;
		size_t sz = 0;
		if (_dupenv_s(&buf, &sz, "EnvVarName") == 0 && buf)
		{
			env = buf;
			free(buf);
		}
#else
		env = getenv("PYTHONPATH");
#endif
		Debug::Warning("Python", "Cannot initialize python! PYTHONPATH env is: " + env);
	}
#ifndef PLATFORM_WIN
	} else {
		Debug::Warning("Python", "Python3.6 framework not loaded!");
	}
#endif
}

bool PyReader::Read(PyScript* scr) {
	Engine::AcquireLock(6);
	//Py_BEGIN_ALLOW_THREADS

	PyScript::ClearLog();
	std::string& path = scr->path;
	std::string mdn = path;
	std::replace(mdn.begin(), mdn.end(), '/', '.');
	std::string spath = IO::path + "nodes/" + path + EXT_PS;
	scr->chgtime = IO::ModTime(spath);
	std::ifstream strm(spath);
	std::string ln;
	if (AnWeb::hasPy) {
		auto mdl = scr->pModule ? PyImport_ReloadModule(scr->pModule) : PyImport_ImportModule(mdn.c_str());
		//Py_DECREF(pName);
		if (!mdl) {
			Debug::Warning("PyReader", "Failed to read python file " + path + EXT_PS "!");
			PyErr_Print();
			goto FAIL;
		}
		scr->pModule = mdl;
		scr->pFunc = PyObject_GetAttrString(scr->pModule, "Execute");
		if (!scr->pFunc || !PyCallable_Check(scr->pFunc)) {
			Debug::Warning("PyReader", "Failed to find \"Execute\" function in " + path + EXT_PS "!");
			Py_XDECREF(scr->pFunc);
			Py_DECREF(scr->pModule);
			goto FAIL;
		}
		Py_INCREF(scr->pFunc);
	}
	//extract io variables
	while (!strm.eof()) {
		std::getline(strm, ln);
		while (ln[0] == '#' && ln[1] == '#' && ln[2] == ' ') {
			scr->desc += ln.substr(3) + "\n";
			scr->descLines++;
			std::getline(strm, ln);
		}

		std::string ln2 = ln.substr(0, 4);
		if (ln2 == "#in ") {
			auto ss = string_split(ln, ' ');
			auto sz = ss.size() - 1;
			for (uint i = 0; i < sz; ++i) {
				scr->_invars.push_back(PyVar());
				auto& bk = scr->_invars.back();
				bk.typeName = ss[i + 1];
				if (!ParseType(bk.typeName, &bk)) {
					Debug::Warning("PyReader::ParseType", "input arg type \"" + bk.typeName + "\" not recognized!");
					goto FAIL;
				}
				scr->invaropts.push_back(VarOpt());
				auto& opt = scr->invaropts.back();
				opt.type = VarOpt::NONE;
			}
			std::getline(strm, ln);
			auto c1 = ln.find_first_of('('), c2 = ln.find_first_of(')');
			if (c1 == std::string::npos || c2 == std::string::npos || c2 <= c1) {
				Debug::Warning("PyReader::ParseType", "braces for input function not found!");
				goto FAIL;
			}
			ss = string_split(ln.substr(c1 + 1, c2 - c1 - 1), ',');
			if (ss.size() != sz) {
				Debug::Warning("PyReader::ParseType", "input function args count not consistent!");
				goto FAIL;
			}
			scr->pArgl = (AnWeb::hasPy) ? PyTuple_New(sz) : nullptr;
			scr->pArgs.resize(sz, 0);
			for (uint i = 0; i < sz; ++i) {
				auto ns = ss[i].find_first_not_of(' ');
				auto ss2 = (ns == std::string::npos) ? ss[i] : ss[i].substr(ns);
				auto tn = scr->_invars[i].typeName;
				scr->_invars[i].name = ss2;
				scr->invars.push_back(std::pair<std::string, std::string>(scr->_invars[i].name, tn));
				if (*((int32_t*)&tn[0]) == *((int32_t*)"list")) {
					//scr->pArgs[i] = AnConv::PyArr(1, tn[6]);
				}
			}
		}
		else if (ln2 == "#out") {
			scr->_outvars.push_back(PyVar());
			auto& bk = scr->_outvars.back();
			bk.typeName = ln.substr(5);
			if (!ParseType(bk.typeName, &bk)) {
				Debug::Warning("PyReader::ParseType", "output arg type \"" + bk.typeName + "\" not recognized!");
				goto FAIL;
			}
			std::getline(strm, ln);
			bk.name = ln.substr(0, ln.find_first_of(' '));
			scr->outvars.push_back(std::pair<std::string, std::string>(bk.name, bk.typeName));
			if (AnWeb::hasPy) scr->pRets.push_back(PyObject_GetAttrString(scr->pModule, bk.name.c_str()));
			else scr->pRets.push_back(nullptr);
		}
	}

	if (!scr->invars.size()) {
		Debug::Warning("PyReader", "Script has no input parameters!");
	}
	if (!scr->outvars.size()) {
		Debug::Warning("PyReader", "Script has no output parameters!");
	}

	//PyEval_RestoreThread(_save);
	Engine::ReleaseLock();
	return true;
FAIL:
	//PyEval_RestoreThread(_save);
	Engine::ReleaseLock();
	return false;
}

void PyReader::Refresh(PyScript* scr) {
	auto mt = IO::ModTime(IO::path + "nodes/" + scr->path + EXT_PS);
	if (mt > scr->chgtime || !scr->ok) {
		AnBrowse::busyMsg = "Reloading " + scr->path + EXT_PS;
		Debug::Message("PyReader", AnBrowse::busyMsg);
		scr->Clear();
		scr->ok = Read(scr);
	}
}

bool PyReader::ParseType(std::string s, PyVar* var) {
	if (s.substr(0, 3) == "int") var->type = AN_VARTYPE::INT;
	else if (s.substr(0, 6) == "double") var->type = AN_VARTYPE::DOUBLE;
	else if (s.substr(0, 4) == "list") {
		var->type = AN_VARTYPE::LIST;
		var->dim = s[5] - '1' + 1;
		var->stride = AnScript::StrideOf(s[6]);
	}
	else return false;
	return true;
}