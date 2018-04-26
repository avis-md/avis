#include "pyreader.h"

void PyScript::Exec() {

}

bool PyReader::Read(string path, PyScript** _scr) {
	auto scr = *_scr = new PyScript();
	auto ls = path.find_last_of("/");
	std::replace(path.begin(), path.end(), '\\', '/');
	string ph = path.substr(0, ls);
	string& nm = scr->name = path.substr(ls + 1);
	nm = nm.substr(0, nm.size() - 3);
	//auto pName = PyUnicode_FromString(path.c_str());
	//scr->pModule = PyImport_Import(pName);
	PyObject *sys_path = PySys_GetObject("path");
	PyList_Append(sys_path, PyUnicode_FromString(ph.c_str()));
	scr->pModule = PyImport_ImportModule(nm.c_str());
	//Py_DECREF(pName);
	if (!scr->pModule) {
		Debug::Warning("PyReader", "Failed to read python file!");
		delete(scr);
		*_scr = nullptr;
		return false;
	}
	scr->pFunc = PyObject_GetAttrString(scr->pModule, "Execute");
	if (!scr->pFunc || !PyCallable_Check(scr->pFunc)) {
		Debug::Warning("PyReader", "Failed to find \"Execute\" function!");
		Py_XDECREF(scr->pFunc);
		Py_DECREF(scr->pModule);
		delete(scr);
		*_scr = nullptr;
		return false;
	}

	//extract io variables
	std::ifstream strm(path);
	string ln;
	while (!strm.eof()) {
		std::getline(strm, ln);
		string ln2 = ln.substr(0, 4);
		if (ln2 == "#out") {
			scr->outvars.push_back(PyVar());
			auto& bk = scr->outvars.back();
			bk.typeName = ln.substr(5);
			ParseType(bk.typeName, &bk);
			scr->outvarCnt++;
			std::getline(strm, ln);
			bk.name = ln.substr(0, ln.find_first_of(' '));
		}
		else if (ln2 == "#in ") {
			auto ss = string_split(ln, ' ');
			scr->invarCnt = ss.size() - 1;
			for (uint i = 0; i < scr->invarCnt; i++) {
				scr->invars.push_back(PyVar());
				auto& bk = scr->invars.back();
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
			if (ss.size() != scr->invarCnt) {
				Debug::Warning("PyReader::ParseType", "input function args count not consistent!");
				return false;
			}
			for (uint i = 0; i < scr->invarCnt; i++) {
				auto ns = ss[i].find_first_not_of(' ');
				auto ss2 = (ns == string::npos) ? ss[i] : ss[i].substr(ns);
				scr->invars[i].name = ss2;
			}
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