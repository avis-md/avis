#include "anweb.h"
#include "vis/system.h"
#ifndef IS_ANSERVER
#include "utils/runcmd.h"
#endif

//#define _GEN_DEBUG

void FReader::Init() {

}

bool FReader::Read(string path, FScript* scr) {
	string fp = IO::path + "/nodes/" + path;
	std::replace(fp.begin(), fp.end(), '\\', '/');
	auto ls = fp.find_last_of('/');
	string nm = fp.substr(ls + 1);
	string fp2 = fp.substr(0, ls + 1) + "__fcache__/";
    
	auto s = IO::GetText(fp + ".f90");

#ifndef IS_ANSERVER
	if (!IO::HasDirectory(fp2)) IO::MakeDirectory(fp2);

	auto mt = scr->chgtime = IO::ModTime(fp + ".f90");
	if (mt < 0) return false;
	auto ot = IO::ModTime(fp2 + nm + ".so");

	string funcNm = "";

	if (mt >= ot) {
		string lp(fp2 + nm + ".so");
		remove(&lp[0]);

		{
			std::ifstream strm(fp + ".f90");
			std::ofstream ostrm(fp + "_temp__.f90");
			string s;
			int tp = 0;
			size_t loc = -1;
			while (std::getline(strm, s)) {
				if (s[0] != '!') {
					switch (tp) {
					case 0:
						ostrm << s << "\n";
						break;
					case 1:
					case 2:
						loc = s.find_first_of(':');
						if (loc < 2) {
							Debug::Warning("FReader", "Variable parse error after \"!in\"!");
							return false;
						}
						if (string_find(to_lowercase(s), ", allocatable") > -1)
							ostrm << s << "\n";
						else
							ostrm << s.substr(0, loc-1) << ", BIND(C) " << s.substr(loc) << "\n";
						break;
					case 3:
						auto sl = string_find(s, "subroutine");
						if (sl == -1) {
							Debug::Warning("FReader", "Subroutine missing after \"!entry\"!");
							return false;
						}
						auto p1 = s.find_first_not_of(' ', sl + 11);
						auto bl = string_find(s, "()", p1 + 1);
						if (sl == -1) {
							Debug::Warning("FReader", "\"()\" missing after \"!entry\"!");
							return false;
						}
						funcNm = s.substr(p1, bl - p1);
						ostrm << s << " BIND(C)\n";
						break;
					}
					tp = 0;
				}
				else {
					if (s == "!in") tp = 1;
					if (s == "!out") tp = 2;
					if (s == "!entry") tp = 3;
				}
			}
		}

#ifdef PLATFORM_WIN

#else
		string cmd = "gfortran -shared -fPIC ";
		cmd += " -o \"" + fp2 + nm + ".so\" \"" + fp + "_temp__.f90\" 2> " + fp + "_log.txt";
		std::cout << cmd << std::endl;
		RunCmd::Run(cmd);
		//scr->errorCount = ErrorView::Parse_GCC(fp + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
#endif
		for (auto& m : scr->compileLog) {
			m.path = fp + ".f90";
		}
		ErrorView::compileMsgs.insert(ErrorView::compileMsgs.end(), scr->compileLog.begin(), scr->compileLog.end());
		ErrorView::compileMsgSz = ErrorView::compileMsgs.size();
	
		remove((fp + "_temp__.f90").c_str());
	}
	else {
		std::ifstream strm(fp + ".f90");
		while (std::getline(strm, s)) {
			if (s == "!entry") {
				std::getline(strm, s);
				auto sl = string_find(s, "subroutine");
				if (sl == -1) {
					Debug::Warning("FReader", "Subroutine missing after \"!entry\"!");
					return false;
				}
				auto p1 = s.find_first_not_of(' ', sl + 11);
				auto bl = string_find(s, "()", p1 + 1);
				if (sl == -1) {
					Debug::Warning("FReader", "\"()\" missing after \"!entry\"!");
					return false;
				}
				funcNm = s.substr(p1, bl - p1);
			}
		}
	}
	
#endif

	scr->name = path;

	if (AnWeb::hasFt) {
		scr->libpath = fp2 + nm + ".so";
		scr->lib = new DyLib(scr->libpath);
		if (!scr->lib->is_open()) {
			Debug::Warning("FReader", "Failed to load script into memory!");
			return false;
		}
		scr->funcLoc = (CScript::emptyFunc)scr->lib->GetSym(to_lowercase(funcNm));
		if (!scr->funcLoc) {
			string err = 
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			Debug::Warning("FReader", "Failed to load function \"" + funcNm + "\" into memory! " + err);
			return false;
		}
	}


	std::ifstream strm(fp + ".f90");
	string ln;
	CVar* bk;
	bool fst = true;
	while (!strm.eof()) {
		std::getline(strm, ln);
		while (fst && ln[0] == '/' && ln[1] == '/' && ln[2] == '/') {
			scr->desc += ln.substr(3) + "\n";
			scr->descLines++;
			std::getline(strm, ln);
		}
		fst = false;

		if (ln == "!out") {
			std::getline(strm, ln);
			auto ss = string_split(ln, ' ', true);
			auto sss = ss.size();
			scr->_outvars.push_back(CVar());
			bk = &scr->_outvars.back();
			int atn = 0;
			while (ss[atn].back() == ',') {
				ss[atn].pop_back();
				atn++;
			}
			bk->typeName = ss[0];
			if (!ParseType(bk->typeName, bk)) {
				Debug::Warning("FReader", "output arg type \"" + bk->typeName + "\" not recognized!");
				return false;
			}
			if (sss < atn + 2) {
				Debug::Warning("FReader", "where is the variable name?");
				return false;
			}
			bk->name = ss[atn + 2];
			bool isa = false;
			if (sss > atn + 3) {
				auto sa = ss[atn + 3];
				auto sas = sa.size();
				if (sa.substr(0, 2) == "(:" && sa.substr(sas-2) == ":)") {
					for (auto c : sa) {
						if (c == ',') bk->dimVals.push_back(new int());
					}
					isa = true;
				}
			}
			
			scr->outvars.push_back(std::pair<string, string>(bk->name, bk->typeName));
			if (AnWeb::hasFt) {
				if (!isa)
					bk->value = scr->lib->GetSym(to_lowercase(bk->name));
				else
					bk->value = scr->lib->GetSym("__test_MOD_" + to_lowercase(bk->name));
				if (!bk->value) {
					Debug::Warning("CReader", "cannot find \"" + bk->name + "\" from memory!");
					return false;
				}
			}
		}
	}


	FScript::allScrs.emplace(path, scr);

	return true;
}

void FReader::Refresh(FScript* scr) {
	auto mt = IO::ModTime(IO::path + "/nodes/" + scr->path + ".f90");
	if (mt > scr->chgtime) {
		Debug::Message("FReader", "Reloading " + scr->path + ".f90");
		scr->Clear();
		scr->ok = Read(scr->path, scr);
	}
}

bool FReader::ParseType(string s, CVar* var) {
	s = to_lowercase(s);
	if (s == "integer") var->type = AN_VARTYPE::INT;
	else if (s == "real*8") var->type = AN_VARTYPE::FLOAT;
	else return false;
	return true;
}