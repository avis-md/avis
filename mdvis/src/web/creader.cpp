#include "anweb.h"
#include "vis/system.h"
#ifndef IS_ANSERVER
#include "utils/runcmd.h"
#endif

string CReader::vcbatPath = "", CReader::mingwPath = "";
bool CReader::useMsvc, CReader::useOMP;
string CReader::flags1, CReader::flags2;

void CReader::Init() {
	flags1 = VisSystem::prefs["ANL_COMP_FLAGS"];
#ifdef PLATFORM_WIN
	vcbatPath = VisSystem::envs["VCBAT"];
	mingwPath = VisSystem::envs["MINGW"];
	useMsvc = (VisSystem::prefs["ANL_WIN_USE_MSVC"] == "true");
	int has = 2;
	if (!mingwPath.size() || !IO::HasFile(mingwPath + "/g++.exe")) {
		mingwPath = "";
		has--;
		if (!useMsvc) useMsvc = true;
	}
	if (!vcbatPath.size() || !IO::HasFile(vcbatPath)) {
		vcbatPath = "";
		has--;
		if (!useMsvc) useMsvc = true;
	}
	if (!!has) {
		AnWeb::hasC = true;
		if (useMsvc) {
			flags1 = VisSystem::prefs["ANL_CL_FLAGS"];
			flags2 = VisSystem::prefs["ANL_LINK_FLAGS"];
		}
	}
#else
	AnWeb::hasC = true;
#endif
	useOMP = (VisSystem::prefs["ANL_USE_OPENMP"] == "true");
}

bool CReader::Read(string path, CScript* scr) {
	scr->name = path;
	string fp = IO::path + "/nodes/" + path;
	std::replace(fp.begin(), fp.end(), '\\', '/');
	auto ls = fp.find_last_of('/');
	string nm = fp.substr(ls + 1);
	string fp2 = fp.substr(0, ls + 1) + "__ccache__/";
    
	auto s = IO::GetText(fp + ".cpp");

#ifndef IS_ANSERVER
	if (!IO::HasDirectory(fp2)) IO::MakeDirectory(fp2);

	if (AnWeb::hasC) {
		auto mt = scr->chgtime = IO::ModTime(fp + ".cpp");
		if (mt < 0) return false;
		auto ot = IO::ModTime(fp2 + nm + ".so");

		if (mt >= ot) {

			string lp(fp2 + nm + ".so");
			remove(&lp[0]);

#ifdef PLATFORM_WIN
			const string dlx = " __declspec(dllexport)";
#else
			const string dlx = "";
#endif
			s = "\
#define VARIN extern \"C\"" + dlx + "\n\
#define VAROUT VARIN\n\
#define ENTRY VARIN void\n\
#define VECSZ(...)\n\
#define VECVAR VARIN\n\
#define PROGRS VARIN\n\
#line 1\n" + s;
			std::ofstream ostrm(fp + "_temp__.cpp");
			ostrm << s;
			ostrm.close();

#ifdef PLATFORM_WIN
			if (useMsvc) {
				//if (0) {
				string cl = "cl /nologo /c -Od ";
				if (useOMP) {
					cl += " /openmp";
				}
				cl += " /EHsc /Fo\"" + fp2 + nm + ".obj\" \"" + fp + "_temp__.cpp\"";
				const string lk = "link /nologo /dll /out:\"" + fp2 + nm + ".so\" \"" + fp2 + nm + ".obj\"";
				RunCmd::Run("\"" + vcbatPath + "\" && " + cl + " > \"" + fp + "_log.txt\" && " + lk + " > \"" + fp + "_log.txt\"");
				scr->errorCount = ErrorView::Parse_MSVC(fp + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
			}
			else {
				string cmd = "g++ -std=c++11 -static-libstdc++ -shared -fPIC " + flags1;
				if (useOMP) {
					cmd += " -fopenmp";
				}
				cmd += " -lm -o \"" + fp2 + nm + ".so\" \"" + fp + "_temp__.cpp\" 2> \"" + fp + "_log.txt\"";
				std::cout << cmd << std::endl;
				RunCmd::Run("path=%path%;" + mingwPath + " && " + cmd);
				scr->errorCount = ErrorView::Parse_GCC(fp + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
			}
#else
			string cmd = "g++ -std=c++11 -shared -fPIC "
#ifdef PLATFORM_LNX
				" -fno-gnu-unique "
#endif
				+ flags1;
			if (useOMP) {
#ifdef PLATFORM_OSX
				cmd += " -Xpreprocessor -fopenmp -lomp";
#else
				cmd += " -fopenmp";
#endif
			}
			cmd += " -lm -o \"" + fp2 + nm + ".so\" \"" + fp + "_temp__.cpp\" 2> " + fp + "_log.txt";
			std::cout << cmd << std::endl;
			RunCmd::Run(cmd);
			scr->errorCount = ErrorView::Parse_GCC(fp + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
#endif
			for (auto& m : scr->compileLog) {
				m.path = fp + ".cpp";
			}
			ErrorView::compileMsgs.insert(ErrorView::compileMsgs.end(), scr->compileLog.begin(), scr->compileLog.end());
			ErrorView::compileMsgSz = ErrorView::compileMsgs.size();
			remove((fp + "_temp__.cpp").c_str());

		}
	
#endif

		scr->libpath = fp2 + nm + ".so";
		scr->lib = new DyLib(scr->libpath);
		if (!scr->lib->is_open()) {
			string err =
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			Debug::Warning("CReader", "Failed to load script into memory! " + err);
			return false;
		}
		scr->funcLoc = (CScript::emptyFunc)scr->lib->GetSym("Execute");
		if (!scr->funcLoc) {
			string err = 
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			Debug::Warning("CReader", "Failed to load function Execute into memory! " + err);
			return false;
		}
	}

	const char* tpnms[] = { "float*", "short*", "int*" };
	const char tpaps[] = { 'f', 's', 'i' };
	const int tpszs[] = { 6, 6, 4 };
	std::ifstream strm(fp + ".cpp");
	string ln;
	std::vector<std::pair<string, int*>> vecvars;
	std::vector<std::pair<string, int**>> vecvarLocs;
	int vec = 0;
	CVar vr;
	bool fst = true;
	while (!strm.eof()) {
		std::getline(strm, ln);
		while (ln[0] == '/' && ln[1] == '/' && ln[2] == '/') {
			scr->desc += ln.substr(3) + "\n";
			scr->descLines++;
			std::getline(strm, ln);
		}
		string ln2 = ln.substr(0, 6);
		long long lln2 = *((long long*)&ln2[0]) & 0x0000ffffffffffff;
#define TL(s) (*((long long*)s) & 0x0000ffffffffffff)
		if (lln2 == TL("VARIN ")) {
			auto ss = string_split(ln, ' ', true);
			if (ss.size() < 3) {
				Debug::Warning("CReader", "incomplete type for VARIN!");
				return false;
			}
			CVar* bk = 0;
			if (vec > 0) {
				auto s2 = rm_spaces(ln.substr(6));
				byte tp = 255;
				for (byte b = 0; b < 3; b++) {
					if (s2.substr(0, tpszs[b]) == tpnms[b]) {
						tp = b;
						break;
					}
				}
				if (tp == 255) {
					Debug::Warning("CReader", "unsupported type for list!");
					return false;
				}
				scr->_invars.push_back(vr);
				bk = &scr->_invars.back();
				bk->name = s2.substr(tpszs[tp], s2.find_first_of('=') - tpszs[tp]).substr(0, ln.find_first_of(';'));
				bk->type = AN_VARTYPE::LIST;
				bk->typeName[6] = tpaps[tp];
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
			if (scr->lib) {
				bk->value = scr->lib->GetSym(bk->name);
				if (!bk->value) {
					Debug::Warning("CReader", "cannot find \"" + bk->name + "\" from memory!");
					return false;
				}
			}
		}
		else if (lln2 == TL("VAROUT")) {
			auto ss = string_split(ln, ' ', true);
			if (ss.size() < 3) {
				Debug::Warning("CReader", "incomplete type for VAROUT!");
				return false;
			}
			CVar* bk = 0;
			if (vec > 0) {
				auto s2 = rm_spaces(ln.substr(6));
				byte tp = 255;
				for (byte b = 0; b < 3; b++) {
					if (s2.substr(0, tpszs[b]) == tpnms[b]) {
						tp = b;
						break;
					}
				}
				if (tp == 255) {
					Debug::Warning("CReader", "unsupported type for list!");
					return false;
				}
				scr->_outvars.push_back(vr);
				bk = &scr->_outvars.back();
				bk->name = s2.substr(tpszs[tp], s2.find_first_of('=') - tpszs[tp]).substr(0, ln.find_first_of(';'));
				bk->type = AN_VARTYPE::LIST;
				bk->typeName[6] = tpaps[tp];
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
			bk->value = (AnWeb::hasC) ? scr->lib->GetSym(bk->name) : nullptr;
			if (!bk->value) {
				Debug::Warning("CReader", "cannot find \"" + bk->name + "\" from memory!");
				return false;
			}
		}
		else if (lln2 == TL("VECVAR")) {
			auto ss = string_split(ln, ' ', true);
			if (ss.size() < 3) {
				Debug::Warning("CReader", "incomplete type for VECVAR!");
				return false;
			}
			if (ss[1] != "int") {
				Debug::Warning("CReader", "is \"" + ln + "\" an int type?");
			}
			auto nm = ss[2].substr(0, ln.find_first_of('=')).substr(0, ln.find_first_of(';'));
			if (scr->lib) {
				void* loc = scr->lib->GetSym(nm);
				if (!loc) {
					Debug::Warning("CReader", "cannot find \"" + nm + "\" from memory!");
					return false;
				}
				vecvars.push_back(std::pair<string, int*>(nm, (int*)loc));
			}
			else
				vecvars.push_back(std::pair<string, int*>(nm, nullptr));
		}
		else if (lln2 == TL("PROGRS")) {
			auto ss = string_split(ln, ' ', true);
			if (ss.size() < 3) {
				Debug::Warning("CReader", "incomplete type for PROGRS!");
				return false;
			}
			if (ss[1] != "float") {
				Debug::Warning("CReader", "PROGRS type must be float!");
				return false;
			}
			auto nm = ss[2].substr(0, ln.find_first_of('=')).substr(0, ln.find_first_of(';'));
			scr->progress = (AnWeb::hasC) ? scr->lib->GetSym(nm) : nullptr;
			if (!scr->progress) {
				Debug::Warning("CReader", "cannot find \"" + nm + "\" from memory!");
				return false;
			}
		}
		if (lln2 == TL("VECSZ(")) {
			auto p1 = 6U;
			auto p2 = ln.find_first_of(')');
			if (p2 < p1) {
				Debug::Warning("CReader", "VECSZ syntax is not correct!");
				return false;
			}
			ln = ln.substr(p1, p2 - p1);
			auto ss = string_split(ln, ',', true);
			vec = ss.size();
			vr = CVar();
			vr.typeName = "list(" + std::to_string(vec) + " )";
			vr.dimNames.resize(vec);
			vr.dimVals.resize(vec);
			int i = 0;
			for (auto& a : ss) {
				a = rm_spaces(a);
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

void CReader::Refresh(CScript* scr) {
	auto mt = IO::ModTime(IO::path + "/nodes/" + scr->path + ".cpp");
	if (mt > scr->chgtime) {
		Debug::Message("CReader", "Reloading " + scr->path + ".cpp");
		scr->Clear();
		scr->ok = Read(scr->path, scr);
	}
}

bool CReader::ParseType(string s, CVar* var) {
	if (s.substr(0, 3) == "int") var->type = AN_VARTYPE::INT;
	else if (s.substr(0, 5) == "float") var->type = AN_VARTYPE::FLOAT;
	else return false;
	return true;
}