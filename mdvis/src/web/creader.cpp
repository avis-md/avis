#include "anweb.h"
#ifndef IS_ANSERVER
#include "utils/runcmd.h"
#endif

#ifdef PLATFORM_WIN
string CReader::vcbatPath = "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\bin\\vcvars32.bat";
#endif

void CReader::Init() {
	
}

string _rm_spaces(string s) {
	string ss;
	ss.reserve(s.size());
	for (char& c : s) {
		if (c != ' ')
			ss += c;
	}
	return ss;
}

bool CReader::Read(string path, CScript** _scr) {
	string fp = IO::path + "/nodes/" + path;
	std::replace(fp.begin(), fp.end(), '\\', '/');
	auto ls = fp.find_last_of('/');
	string nm = fp.substr(ls + 1);
	string fp2 = fp.substr(0, ls + 1) + "__ccache__/";
   
	auto s = IO::GetText(fp + ".cpp");

#ifndef IS_ANSERVER

	if (!IO::HasDirectory(fp2)) IO::MakeDirectory(fp2);
#ifdef PLATFORM_WIN
	const string dlx = " __declspec(dllexport)";
#else
	const string dlx = "";
#endif
	s = "\
#define VARIN extern \"C\"" + dlx + "\n\
#define VAROUT VARIN\n\
#define ENTRY VARIN\n\
#define VECSZ(...)\n\
#define VECVAR VARIN\n\
\n" + s;
	std::ofstream ostrm(fp + "_temp__.cpp");
	ostrm << s;
	ostrm.close();

#ifdef PLATFORM_WIN
	//&& cl /nologo /c /FoD:/ D:/lib.c && link /dll /nologo /out:D:/lib.dll D:/lib.obj"
	const string cl = "cl /nologo /c -Od /EHsc /Fo\"" + fp2 + nm + ".obj\" \"" + fp + "_temp__.cpp\"";
	const string lk = "link /nologo /dll /out:\"" + fp2 + nm + ".dll\" \"" + fp2 + nm + ".obj\"";
	RunCmd::Run("\"" + vcbatPath + "\" && " + cl + " && " + lk);
#else
	const string cmd = "g++ -std=c++11 -shared -O0 -fPIC -o \"" + fp2 + nm + ".so\" \"" + fp + "_temp__.cpp\"";
	//const string cl = "g++ /nologo /c -Od /EHsc /Fo\"" + fp2 + nm + ".obj\" \"" + fp + "_temp__.cpp\"";
	//const string lk = "link /nologo /dll /out:\"" + fp2 + nm + ".dll\" \"" + fp2 + nm + ".obj\"";
	std::cout << cmd << std::endl;
	RunCmd::Run(cmd);
#endif
	const string ss = fp + "_temp__.cpp";
	remove(&ss[0]);

#endif

	auto scr = *_scr = new CScript();
	scr->name = path;
	scr->lib = new DyLib(fp2 + nm + 
#ifdef PLATFORM_WIN
	".dll"
#else
	".so"
#endif
	);
	if (!scr->lib) {
		Debug::Warning("CReader", "Failed to load script into memory!");
		return false;
	}
	scr->funcLoc = (CScript::emptyFunc)scr->lib->GetSym("Execute");
	if (!scr->funcLoc) {
		Debug::Warning("CReader", "Failed to load function Execute into memory!");
		return false;
	}

	std::ifstream strm(fp + ".cpp");
	string ln;
	std::vector<std::pair<string, int*>> vecvars;
	std::vector<std::pair<string, int**>> vecvarLocs;
	int vec = 0;
	CVar vr;
	while (!strm.eof()) {
		std::getline(strm, ln);
		string ln2 = ln.substr(0, 6);
		if (ln2 == "VARIN ") {
			auto ss = string_split(ln, ' ', true);
			if (ss.size() < 3) {
				Debug::Warning("CReader", "incomplete type for VARIN!");
				return false;
			}
			CVar* bk = 0;
			if (vec > 0) {
				auto s2 = _rm_spaces(ln.substr(6));
				if (s2.substr(0, 6) != "float*") {
					Debug::Warning("CReader", "list VARIN must be float*!");
					return false;
				}
				scr->_invars.push_back(vr);
				bk = &scr->_invars.back();
				bk->type = AN_VARTYPE::LIST;
				bk->name = s2.substr(6, s2.find_first_of('=') - 6).substr(0, ln.find_first_of(';'));
				int ii = 0;
				for (auto& a : bk->dimNames) {
					if (!!a[0]) vecvarLocs.push_back(std::pair<string, int**>(a, &bk->dimVals[ii]));
					ii++;
				}
			}
			else {
				scr->_invars.push_back(CVar());
				bk = &scr->_invars.back();
				bk->typeName = ss[1];
				if (!ParseType(bk->typeName, bk)) {
					Debug::Warning("CReader", "input arg type \"" + bk->typeName + "\" not recognized!");
					return false;
				}
				bk->name = ss[2].substr(0, ln.find_first_of('=')).substr(0, ln.find_first_of(';'));
			}
			scr->invars.push_back(std::pair<string, string>(bk->name, bk->typeName));
			bk->value = scr->lib->GetSym(bk->name);
			if (!bk->value) {
				Debug::Warning("CReader", "cannot find \"" + bk->name + "\" from memory!");
				return false;
			}
		}
		else if (ln2 == "VAROUT") {
			auto ss = string_split(ln, ' ', true);
			if (ss.size() < 3) {
				Debug::Warning("CReader", "incomplete type for VAROUT!");
				return false;
			}
			CVar* bk = 0;
			if (vec > 0) {
				auto s2 = _rm_spaces(ln.substr(6));
				if (s2.substr(0, 6) != "float*") {
					Debug::Warning("CReader", "list VAROUT must be float*!");
					return false;
				}
				scr->_outvars.push_back(vr);
				bk = &scr->_outvars.back();
				bk->name = s2.substr(6, s2.find_first_of('=') - 6).substr(0, ln.find_first_of(';'));
				int ii = 0;
				for (auto& a : bk->dimNames) {
					if (!!a[0]) vecvarLocs.push_back(std::pair<string, int**>(a, &bk->dimVals[ii]));
					ii++;
				}
			}
			else {
				scr->_outvars.push_back(CVar());
				bk = &scr->_outvars.back();
				bk->typeName = ss[1];
				if (!ParseType(bk->typeName, bk)) {
					Debug::Warning("CReader", "output arg type \"" + bk->typeName + "\" not recognized!");
					return false;
				}
				bk->name = ss[2].substr(0, ln.find_first_of('=')).substr(0, ln.find_first_of(';'));
			}
			scr->outvars.push_back(std::pair<string, string>(bk->name, bk->typeName));
			bk->value = scr->lib->GetSym(bk->name);
			if (!bk->value) {
				Debug::Warning("CReader", "cannot find \"" + bk->name + "\" from memory!");
				return false;
			}
		}
		else if (ln2 == "VECVAR") {
			auto ss = string_split(ln, ' ', true);
			if (ss.size() < 3) {
				Debug::Warning("CReader", "incomplete type for VECVAR!");
				return false;
			}
			if (ss[1] != "int") {
				Debug::Warning("CReader", "is \"" + ln + "\" an int type?");
			}
			auto nm = ss[2].substr(0, ln.find_first_of('=')).substr(0, ln.find_first_of(';'));
			void* loc = scr->lib->GetSym(nm);
			if (!loc) {
				Debug::Warning("CReader", "cannot find \"" + nm + "\" from memory!");
				return false;
			}
			vecvars.push_back(std::pair<string, int*>(nm, (int*)loc));
		}
		if (ln2 == "VECSZ(") {
			auto p1 = 6;
			auto p2 = ln.find_first_of(')');
			if (p2 < p1) {
				Debug::Warning("CReader", "VECSZ syntax is not correct!");
				return false;
			}
			ln = ln.substr(p1, p2 - p1);
			auto ss = string_split(ln, ',', true);
			vec = ss.size();
			vr = CVar();
			vr.typeName = "list(" + std::to_string(vec) + ")";
			vr.dimNames.resize(vec);
			vr.dimVals.resize(vec);
			int i = 0;
			for (auto& a : ss) {
				a = _rm_spaces(a);
				if (a[0] >= 'A') {
					vr.dimNames[i] = a = a.substr(0, ln.find_first_of('=')).substr(0, ln.find_first_of(';'));
					//vecvarLocs.push_back(std::pair<string, int**>(a, &vr.dimVals[i]));
				}
				else
					vr.dimVals[i] = new int(TryParse(a, 0));
				i++;
			}
		}
		else {
			vec = 0;
		}
	}

	for (auto& v : vecvars) {
		for (int i = vecvarLocs.size() - 1; i >= 0; i--) {
			if (v.first == vecvarLocs[i].first) {
				*vecvarLocs[i].second = v.second;
				vecvarLocs.erase(vecvarLocs.begin() + i);
			}
		}
	}

	if (!scr->outvars.size()) {
		Debug::Warning("CReader", "Script has no output parameters!");
	}

	CScript::allScrs.emplace(path, scr);

	return true;
}

bool CReader::ParseType(string s, CVar* var) {
	if (s.substr(0, 3) == "int") var->type = AN_VARTYPE::INT;
	else if (s.substr(0, 5) == "float") var->type = AN_VARTYPE::FLOAT;
	else return false;
	return true;
}