#include "anweb.h"

void PyReader::Init() {
	if (IO::HasFile(IO::path + "/env.txt")) {
		auto pth = IO::GetText(IO::path + "/env.txt");
		_putenv_s("PYTHONPATH", pth.c_str());
	}
	Py_Initialize();
	PyObject *sys_path = PySys_GetObject((char*)"path");
	auto path = IO::path + "/nodes/";
	std::replace(path.begin(), path.end(), '\\', '/');
	PyList_Append(sys_path, PyUnicode_FromString(path.c_str()));
}

bool PyReader::Read(string path, PyScript** _scr) { //"path/to/script[no .py]"
	auto scr = *_scr = new PyScript();
	scr->name = path;
	string mdn = path;
	std::replace(mdn.begin(), mdn.end(), '/', '.');
	//auto pName = PyUnicode_FromString(path.c_str());
	//scr->pModule = PyImport_Import(pName);
	scr->pModule = PyImport_ImportModule(mdn.c_str());
	//Py_DECREF(pName);
	if (!scr->pModule) {
		Debug::Warning("PyReader", "Failed to read python file " + path + "!");
		PyErr_Print();
		delete(scr);
		*_scr = nullptr;
		return false;
	}
	scr->pFunc = PyObject_GetAttrString(scr->pModule, "Execute");
	if (!scr->pFunc || !PyCallable_Check(scr->pFunc)) {
		Debug::Warning("PyReader", "Failed to find \"Execute\" function in " + path + "!");
		Py_XDECREF(scr->pFunc);
		Py_DECREF(scr->pModule);
		delete(scr);
		*_scr = nullptr;
		return false;
	}
	Py_INCREF(scr->pFunc);

	//extract io variables
	std::ifstream strm(IO::path + "/py/" + path + ".py");
	string ln;
	while (!strm.eof()) {
		std::getline(strm, ln);
		string ln2 = ln.substr(0, 4);
		if (ln2 == "#out") {
			scr->_outvars.push_back(PyVar());
			auto& bk = scr->_outvars.back();
			bk.typeName = ln.substr(5);
			ParseType(bk.typeName, &bk);
			std::getline(strm, ln);
			bk.name = ln.substr(0, ln.find_first_of(' '));
			scr->outvars.push_back(std::pair<string, string>(bk.name, bk.typeName));
			scr->pRets.push_back(PyObject_GetAttrString(scr->pModule, bk.name.c_str()));
		}
		else if (ln2 == "#in ") {
			auto ss = string_split(ln, ' ');
			auto sz = ss.size() - 1;
			for (uint i = 0; i < sz; i++) {
				scr->_invars.push_back(PyVar());
				auto& bk = scr->_invars.back();
				bk.typeName = ss[i + 1];
				ParseType(bk.typeName, &bk);
			}
			std::getline(strm, ln);
			auto c1 = ln.find_first_of('('), c2 = ln.find_first_of(')');
			if (c1 == string::npos || c2 == string::npos || c2 <= c1) {
				Debug::Warning("PyReader::ParseType", "braces for input function not found!");
				return false;
			}
			ss = string_split(ln.substr(c1 + 1, c2 - c1 - 1), ',');
			if (ss.size() != sz) {
				Debug::Warning("PyReader::ParseType", "input function args count not consistent!");
				return false;
			}
			for (uint i = 0; i < sz; i++) {
				auto ns = ss[i].find_first_not_of(' ');
				auto ss2 = (ns == string::npos) ? ss[i] : ss[i].substr(ns);
				scr->_invars[i].name = ss2;
				scr->invars.push_back(std::pair<string, string>(scr->_invars[i].name, scr->_invars[i].typeName));
			}
			scr->pArgs = PyTuple_New(sz);
		}
	}

	if (!scr->outvars.size()) {
		Debug::Warning("PyReader", "Script has no output parameters!");
	}

	return true;
}

bool PyReader::ParseType(string s, PyVar* var) {
	if (s == "int") var->type = PY_VARTYPE::INT;
	else if (s == "float") var->type = PY_VARTYPE::FLOAT;
	else {
		string s2 = s.substr(0, 4);
		auto c1 = s.find_first_of('(');
		if (c1 != 4) {
			Debug::Warning("PyReader::ParseType", "Opening braces for type not found!");
			return false;
		}
		auto c2 = s.size() - 1;
		if (s[c2] != ')') {
			Debug::Warning("PyReader::ParseType", "Closing braces for type not found!");
			return false;
		}
		if (s2 == "list") {
			var->type = PY_VARTYPE::LIST;
			var->child1 = new PyVar();
			ParseType(s.substr(c1 + 1, c2 - c1 - 1), var->child1);
		}
		else if (s2 == "pair") {
			var->type = PY_VARTYPE::PAIR;
			uint cm = 0, ct = 0;
			for (size_t i = 5; i < s.size(); i++) {
				char c = s[i];
				if (c == '(')
					ct++;
				else if (c == ')')
					ct--;
				else if (c == ',' && !ct) {
					cm = i;
					break;
				}
			}
			if (!cm) {
				Debug::Warning("PyReader::ParseType", "Separator for pair not found!");
				return false;
			}
			var->child1 = new PyVar();
			var->child2 = new PyVar();
			ParseType(s.substr(c1 + 1, cm - c1 - 1), var->child1);
			ParseType(s.substr(cm + 1, c2 - cm - 1), var->child2);
		}
		else {
			Debug::Warning("PyReader::ParseType", "Type not supported: \"" + s + "\"!");
			return false;
		}
	}
	return true;
}