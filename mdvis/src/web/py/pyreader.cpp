#include "pyreader.h"
#include "pyarr.h"
#include "vis/system.h"
#include "vis/preferences.h"
#ifndef PLATFORM_WIN
#include <dlfcn.h>
#endif

bool PyReader::initd = false;

void PyReader::Init() {
	initd = true;
#ifndef PLATFORM_WIN
	if (dlsym(RTLD_DEFAULT, "Py_Initialize")) {
#else
	static std::string pyenv;
	Preferences::LinkEnv("PYENV", &pyenv);
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
#endif
	try {
		Py_Initialize();
		PyObject *sys_path = PySys_GetObject("path");
		auto path = AnWeb::nodesPath;
		auto ps = PyUnicode_FromString(path.c_str());
		PyList_Insert(sys_path, 0, ps);
		Py_DecRef(ps);

		if (!!PyArr::Init())
			return;

		AnWeb::hasPy = true;
		//PyScript::InitLog();
		Unloader::Reg(Deinit);
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

void PyReader::Deinit() {
	Py_Finalize();
}

size_t find_first_not_name_char (const char* c) {
	auto c0 = c;
	while (*c) {
		if (*c == '=' || *c < 48 || *c > 122)
			return c - c0;
		++c;
	}
	return -1;
}

bool PyReader::Read(PyScript* scr) {
	Engine::AcquireLock(6);
	//Py_BEGIN_ALLOW_THREADS

	//PyScript::ClearLog();
	std::string& path = scr->path;
	std::string mdn = path;
	std::replace(mdn.begin(), mdn.end(), '/', '.');
	std::string spath = AnWeb::nodesPath + path + EXT_PS;
	scr->chgtime = IO::ModTime(spath);
	std::ifstream strm(spath);
	std::string ln;
	if (AnWeb::hasPy) {
		scr->lib = scr->lib ? PyImport_ReloadModule(scr->lib) : PyImport_ImportModule(mdn.c_str());
		//Py_DECREF(pName);
		if (!scr->lib) {
			Debug::Warning("PyReader", "Failed to read python file " + path + EXT_PS "!");
			PyErr_Print();
			goto FAIL;
		}
		scr->spawner = PyObject_GetAttrString(scr->lib, scr->name.c_str());
		if (!scr->lib) {
			Debug::Warning("PyReader", "Python file" + path + EXT_PS " does not have a \"" + scr->name + "\" class!");
			PyErr_Print();
			goto FAIL;
		}
	}
	//extract io variables
	while (!strm.eof()) {
		std::getline(strm, ln);
		while (ln[0] == '#' && ln[1] == '#' && ln[2] == ' ') {
			scr->desc += ln.substr(3) + "\n";
			scr->descLines++;
			std::getline(strm, ln);
		}

		ln = string_trim(ln);

		if (ln.substr(0, 6) == "#entry") {
			std::getline(strm, ln);
			if (ln.substr(0, 4) != "def ") {
				Debug::Warning("PyReader::ParseType", "#entry expects a function definition!");
				goto FAIL;
			}
			auto c1 = ln.find_first_of('(');
			if (c1 == std::string::npos) {
				Debug::Warning("PyReader::ParseType", "Braces for main function not found!");
				goto FAIL;
			}
			if (ln[c1+1] != ')') {
				Debug::Warning("PyReader::ParseType", "Main function must have no arguments!");
				goto FAIL;
			}
			if (AnWeb::hasPy) {
				scr->funcNm = ln.substr(4, c1 - 4);
			}
		}
		else if (ln.substr(0, 4) == "#in ") {
			scr->inputs.push_back(AnScript::Var());
			scr->_inputs.push_back(PyVar());
			auto& in = scr->inputs.back();
			auto& _in = scr->_inputs.back();
			in.typeName = ln.substr(4);
			if (!ParseType(in)) {
				Debug::Warning("PyReader::ParseType", "input arg type \"" + in.typeName + "\" not recognized!");
				goto FAIL;
			}
			std::getline(strm, ln);
			in.name = AnWeb::ConvertName(
				(_in.name = ln.substr(0, find_first_not_name_char(ln.c_str()))));
			_in.szs.resize(in.dim, -1);
		}
		else if (ln.substr(0, 5) == "#out ") {
			scr->outputs.push_back(AnScript::Var());
			scr->_outputs.push_back(PyVar());
			auto& out = scr->outputs.back();
			auto& _out = scr->_outputs.back();
			out.typeName = ln.substr(4);
			if (!ParseType(out)) {
				Debug::Warning("PyReader::ParseType", "input arg type \"" + out.typeName + "\" not recognized!");
				goto FAIL;
			}
			std::getline(strm, ln);
			out.name = AnWeb::ConvertName(
				(_out.name = ln.substr(0, find_first_not_name_char(ln.c_str()))));
			_out.szs.resize(out.dim, -1);
		}
	}

	//if (!scr->invars.size()) {
	//	Debug::Warning("PyReader", "Script has no input parameters!");
	//}
	if (!scr->outputs.size()) {
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
	auto mt = IO::ModTime(AnWeb::nodesPath + scr->path + EXT_PS);
	if (mt > scr->chgtime || !scr->ok) {
		AnBrowse::busyMsg = "Reloading " + scr->path + EXT_PS;
		Debug::Message("PyReader", AnBrowse::busyMsg);
		scr->Clear();
		scr->ok = Read(scr);
	}
}

bool PyReader::ParseType(AnScript::Var& var) {
	const auto& s = var.typeName;
	if (s.substr(0, 3) == "int") var.type = AN_VARTYPE::INT;
	else if (s.substr(0, 6) == "double") var.type= AN_VARTYPE::DOUBLE;
	else if (s.substr(0, 5) == "list("
			&& s[7] == ')') {
		var.type = AN_VARTYPE::LIST;
		switch (s[6]) {
		case 'i':
			var.itemType = AN_VARTYPE::INT;
			var.stride = 4;
			break;
		case 'd':
			var.itemType = AN_VARTYPE::DOUBLE;
			var.stride = 8;
			break;
		default:
			return false;
		}
		var.dim = s[5] - '1' + 1;
	}
	else return false;
	return true;
}