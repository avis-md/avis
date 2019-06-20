#include "pyreader.h"
#include "pyarr.h"
#include "vis/system.h"
#include "vis/preferences.h"
#ifndef PLATFORM_WIN
#include <dlfcn.h>
#include "utils/runcmd.h"
#endif

bool PyReader::initd = false;
PyObject* PyReader::mainModule;

void PyReader::Init() {
	initd = true;
#ifdef PLATFORM_WIN
	const auto donotloadpath = VisSystem::localFd + "donotloadpython";
	if (IO::HasFile(donotloadpath)) {
		Debug::Warning("PyReader", "Skipping Python initialization because of failure on a previous run.");
		Debug::Warning("PyReader", "Remove \"" + donotloadpath + "\" to retry.");
		return;
	}
	IO::WriteFile(donotloadpath, "");
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
#else
	if (dlsym(RTLD_DEFAULT, "Py_Initialize")) {
#endif
	try {
		Py_Initialize();
		wchar_t arg = 0;
		wchar_t* args[] = { &arg };
		PySys_SetArgvEx(1, args, 0);
		PyObject *sys_path = PySys_GetObject("path");
		auto path = AnWeb::nodesPath;
		auto ps = PyUnicode_FromString(path.c_str());
		PyList_Insert(sys_path, 0, ps);
		Py_DecRef(ps);

#ifdef PLATFORM_WIN
		remove(donotloadpath.c_str());
#endif

		if (!!PyArr::Init())
			return;

		auto plt = PyImport_ImportModule("matplotlib.pyplot");
		if (plt) {
			Debug::Message("PyReader", "Redirecting pyplot output...");
			PyRun_SimpleString(R"(
import matplotlib.pyplot as plt

__cpath_prefix__ = ""
__plt_figcount__ = 0

def __myshowplot__():
	global __cpath_prefix__, __plt_figcount__
	p = (str(__cpath_prefix__, 'utf-8') + "_figure_{}.png").format(plt.gcf().number)
	print("saving figure to: " + p)
	plt.savefig(p, bbox_inches='tight', pad_inches=0)
	__plt_figcount__ += 1;

plt.show = __myshowplot__
)");
		}
		mainModule = PyImport_AddModule("__main__");

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
		Debug::Warning("Python", "Python" PY_VERSION " framework not loaded!");
#ifdef PLATFORM_OSX
		remove((IO::path + "/Python").c_str());
		RunCmd::Run("test -e $(python3.7-config --exec)/Python && "
			"ln -s $(python3.7-config --exec)/Python \"" + IO::path + "/Python\"");
		if (IO::HasFile(IO::path + "/Python")) {
			Debug::Warning("Python", "Python link (re)created. Restarting the app may fix the problem.");
		}
#endif
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
	Engine::Locker _lock(6);
	std::string mdn = scr->path;
	std::replace(mdn.begin(), mdn.end(), '/', '.');
	std::string spath = AnWeb::nodesPath + scr->path + EXT_PS;
	scr->chgtime = IO::ModTime(spath);
	if (AnWeb::hasPy) {
		scr->lib = scr->lib ? PyImport_ReloadModule(scr->lib) : PyImport_ImportModule(mdn.c_str());
		if (!scr->lib) {
			Debug::Warning("PyReader", "Failed to import python module " + scr->path + EXT_PS "!");
			PyErr_Print();
			return false;
		}
	}
	auto res = PyReader::ReadClassed(scr, spath);
	if (res >= 0) return !res;
	else {
		Debug::Message("PyReader", "Attempting to read static version of python module");
		return (!PyReader::ReadStatic(scr, spath));
	}
}

int PyReader::ReadClassed(PyScript* scr, const std::string spath) {
	//PyScript::ClearLog();
	scr->isSingleton = false;
	std::ifstream strm(spath);
	std::string ln;
	//extract io variables
	int checkinit = 2;
	bool candecl = true;
	while (!strm.eof()) {
		ParseDesc(strm, ln, scr);

		if (checkinit > 0) {
			ln = rm_spaces(ln);
			if (checkinit == 2) {
				if (ln == "class" + scr->name + ":")
					checkinit--;
			}
			else {
				if (ln == "def__init__(self):") {
					checkinit--;
					candecl = true;
				}
			}
		}
		else {
			ln = string_trim(ln);

			if (ln[0] != '#') {
				if (candecl && ln.substr(0, 4) == "def ") {
					candecl = false;
				}
				else continue;
			}
			if (ln.substr(1) == "entry") {
				std::getline(strm, ln);
				ln = string_trim(ln);
				if (ln.substr(0, 4) != "def ") {
					Debug::Warning("PyReader::Parse", "#entry expects a function definition!");
					return 11;
				}
				auto c1 = ln.find_first_of('(');
				if (c1 == std::string::npos) {
					Debug::Warning("PyReader::Parse", "Braces for main function not found!");
					return 12;
				}
				if (ln.substr(c1 + 1, 5) != "self)") {
					Debug::Warning("PyReader::Parse", "Main function must have a single \'self\' argument!");
					return 13;
				}
				if (AnWeb::hasPy) {
					scr->funcNm = ln.substr(4, c1 - 4);
				}
			}
			else if (ln.substr(1, 3) == "in ") {
				if (!candecl) {
					Debug::Warning("PyReader::Parse", "Input variable must be defined inside the __init__ function!");
					continue;
				}
				if (!ParseVar(strm, ln, scr, true, true))
					return 20;
			}
			else if (ln.substr(1, 4) == "out ") {
				if (!candecl) {
					Debug::Warning("PyReader::Parse", "Output variable must be defined inside the __init__ function!");
					continue;
				}
				if (!ParseVar(strm, ln, scr, false, true))
					return 30;
			}
		}
	}
	if (checkinit == 2) {
		Debug::Warning("PyReader::Parse", "Cannot find '" + scr->name + "' class!");
		return -1;
	}
	else if (checkinit == 1) {
		Debug::Warning("PyReader::Parse", "Cannot find __init__ function!");
		return 1;
	}
	if (AnWeb::hasPy) {
		scr->spawner = PyObject_GetAttrString(scr->lib, scr->name.c_str());
		if (!scr->spawner) {
			Debug::Warning("PyReader", "Failed to load '" + scr->name + "' class attribute!");
			PyErr_Print();
			return 5;
		}
	}

	return 0;
}

int PyReader::ReadStatic(PyScript* scr, const std::string spath) {
	//PyScript::ClearLog();
	scr->isSingleton = true;
	std::ifstream strm(spath);
	std::string ln;
	
	while (!strm.eof()) {
		ParseDesc(strm, ln, scr);

		if (ln[0] == '#') {
			if (ln.substr(1) == "entry") {
				std::getline(strm, ln);
				ln = string_trim(ln);
				if (ln.substr(0, 4) != "def ") {
					Debug::Warning("PyReader::Parse", "#entry expects a function definition!");
					return 11;
				}
				auto c1 = ln.find_first_of('(');
				if (c1 == std::string::npos) {
					Debug::Warning("PyReader::Parse", "Braces for main function not found!");
					return 12;
				}
				if (ln[c1 + 1] != ')') {
					Debug::Warning("PyReader::Parse", "Main function must have no arguments!");
					return 13;
				}
				if (AnWeb::hasPy) {
					scr->funcNm = ln.substr(4, c1 - 4);
				}
			}
			else if (ln.substr(1, 3) == "in ") {
				if (!ParseVar(strm, ln, scr, true, false))
					return 20;
			}
			else if (ln.substr(1, 4) == "out ") {
				if (!ParseVar(strm, ln, scr, false, false))
					return 30;
			}
		}
	}

	return 0;
}

void PyReader::Refresh(PyScript* scr) {
	auto mt = IO::ModTime(AnWeb::nodesPath + scr->path + EXT_PS);
	if ((mt > scr->chgtime) || (!scr->ok && mt > scr->badtime)) {
		AnBrowse::busyMsg = "Reloading " + scr->path + EXT_PS;
		Debug::Message("PyReader", AnBrowse::busyMsg);
		if (scr->ok) scr->UnregInstances();
		scr->Clear();
		scr->ok = Read(scr);
		if (scr->ok) scr->RegInstances();
	}
}

void PyReader::ParseDesc(std::istream& strm, std::string& ln, PyScript* scr) {
	scr->desc = "";
	scr->descLines = 0;
	std::getline(strm, ln);
	while (ln[0] == '#' && ln[1] == '#' && ln[2] == ' ') {
		scr->desc += ln.substr(3) + "\n";
		scr->descLines++;
		std::getline(strm, ln);
	}
}

bool PyReader::ParseVar(std::istream& strm, std::string& ln, PyScript* scr, bool in, bool self) {
	auto& vs = in? scr->inputs : scr->outputs;
	auto& _vs = in? scr->_inputs : scr->_outputs;
	vs.push_back(AnScript::Var());
	_vs.push_back(PyVar());
	auto& v = vs.back();
	auto& _v = _vs.back();
	v.typeName = in? ln.substr(4) : ln.substr(5);
	if (!ParseType(v)) {
		Debug::Warning("PyReader::ParseType", "Arg type \"" + v.typeName + "\" not recognized!");
		return false;
	}
	std::getline(strm, ln);
	ln = string_trim(ln);
	if (self){
		if (ln.substr(0, 5) != "self.") {
			Debug::Warning("PyReader::ParseType", "`self.` expected before variable!");
			return false;
		}
		ln = ln.substr(5);
	}
	v.name = AnWeb::ConvertName((_v.name
		= ln.substr(0, find_first_not_name_char(ln.c_str()))));
	_v.szs.resize(v.dim, -1);
	return true;
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
