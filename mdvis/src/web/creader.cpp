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

		string funcNm;

		bool fail = false;
#define _ER(a, b)\
			scr->compileLog.push_back(ErrorView::Message());\
			auto& msgg = scr->compileLog.back();\
			msgg.name = path;\
			msgg.path = fp + ".cpp";\
			msgg.msg.resize(1, "(System) " b);\
			msgg.severe = true;\
			scr->errorCount = 1;\
			ErrorView::compileMsgs.push_back(msgg);\
			ErrorView::compileMsgSz++;\
			Debug::Warning(a, b);

		if (mt > ot || !IO::HasFile(fp2 + nm + ".entry")) {
			remove((fp2 + nm + ".so").c_str());

#ifdef PLATFORM_WIN
			const string dlx = " __declspec(dllexport)";
#else
			const string dlx = "";
#endif
			{
				std::ifstream strm(fp + ".cpp");
				std::ofstream ostrm(fp + "_temp__.cpp");
				string s;
				int tp = 0;
				size_t loc = -1;
				while (std::getline(strm, s)) {
					if (s[0] == '/' && s[1] == '/') {
						auto ss = string_split(s, ' ');
						if (ss[0] == "//in"
							|| ss[0] == "//out"
							|| ss[0] == "//var") {
							ostrm << "extern \"C\" " + dlx << '\n';
						}
						else if (ss[0] == "//entry") {
							ostrm << "extern \"C\" " + dlx << '\n';
							std::getline(strm, s);
							ostrm << s << '\n';
							s = rm_spaces(s);
							int bo = s.find('(');
							int bc = s.find(')');
							string ib = s.substr(bo + 1, bc - bo - 1);
							if ((*(int32_t*)&s[0] == *(int32_t*)"void") && (ib == "" || ib == "void")) {
								funcNm = s.substr(4, bo - 4);
								std::ofstream ets(fp2 + nm + ".entry");
								ets << funcNm;
							}
							else {
								_ER("CReader", "//entry must precede a void function with no arguments!");
								fail = true;
								break;
							}
						}
						else ostrm << s << "\n";
					}
					else ostrm << s << "\n";
				}
				if (funcNm == "") {
					_ER("CReader", "Script has no entry point!");
					fail = true;
				}
			}

			if (fail) {
				remove((fp + "_temp__.cpp").c_str());
				return false;
			}

#ifdef PLATFORM_WIN
			if (useMsvc) {
				string cl = "cl /nologo /c -Od ";
				if (useOMP) {
					cl += " /openmp";
				}
				cl += " /EHsc /Fo\"" + fp2 + nm + ".obj\" \"" + fp + "_temp__.cpp\"";
				const string lk = "link /nologo /dll /out:\"" + fp2 + nm + ".so\" \"" + fp2 + nm + ".obj\"";
				RunCmd::Run("\"" + vcbatPath + "\" && " + cl + " > \"" + fp2 + nm + "_log.txt\" && " + lk + " > \"" + fp2 + nm + "_log.txt\"");
				scr->errorCount = ErrorView::Parse_MSVC(fp2 + nm + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
			}
			else {
				string cmd = "g++ -std=c++11 -static-libstdc++ -shared -fPIC " + flags1;
				if (useOMP) {
					cmd += " -fopenmp";
				}
				cmd += " -lm -o \"" + fp2 + nm + ".so\" \"" + fp + "_temp__.cpp\" 2> \"" + fp2 + nm + "_log.txt\"";
				std::cout << cmd << std::endl;
				RunCmd::Run("path=%path%;" + mingwPath + " && " + cmd);
				scr->errorCount = ErrorView::Parse_GCC(fp2 + nm + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
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
			ErrorView::compileMsgSz += ErrorView::compileMsgs.size();
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
			_ER("CReader", "Failed to load script into memory! " + err);
			return false;
		}
		{
			std::ifstream ets(fp2 + nm + ".entry");
			ets >> funcNm;
		}
		scr->funcLoc = (emptyFunc)scr->lib->GetSym(funcNm);
		if (!scr->funcLoc) {
			string err = 
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			_ER("CReader", "Failed to load function \"" + funcNm + "\" into memory! " + err);
			return false;
		}
	}

	std::ifstream strm(fp + ".cpp");
	string ln;
	bool fst = true;
	while (!strm.eof()) {
		std::getline(strm, ln);
		while (fst && ln[0] == '/' && ln[1] == '/' && ln[2] == '/') {
			scr->desc += ln.substr(3) + "\n";
			scr->descLines++;
			std::getline(strm, ln);
		}
		fst = false;

		auto lns = string_split(ln, ' ', true);
		int lnsz = lns.size();
		if (!lnsz) continue;
		bool iso = (lns[0] == "//out");
		if (lns[0] == "//in" || iso) {
			std::vector<std::pair<string, string>>& cv = iso ? scr->outvars : scr->invars;
			std::vector<CVar>& _cv = iso ? scr->_outvars : scr->_invars;
			const string ios = iso ? "output " : "input ";
			_cv.push_back(CVar());
			auto bk = &_cv.back();

			std::getline(strm, ln);
			bool ira = false;

			auto ss = string_split(ln, ' ', true);
			bk->typeName = ss[0];
			if (bk->typeName.back() == '*') {
				bk->typeName.pop_back();
				ira = true;
			}
			else if (ss[1][0] == '*') {
				ss[1] = ss[1].substr(1);
				ira = true;
			}
			if (ira && lnsz == 1) {
				_ER("CReader", "Plain " + ios + " must be of non-pointer type!");
				return false;
			}
			if (!ira && lnsz > 1) {
				_ER("CReader", "" + ios + " with specified size must be of pointer type!");
				return false;
			}
			if (!ira && !ParseType(bk->typeName, bk)) {
				_ER("CReader", "arg type \"" + bk->typeName + "\" not recognized!");
				return false;
			}
			
			string::iterator eps;
			if ((eps = std::find(ss[1].begin(), ss[1].end(), '=')) != ss[1].end()) {
				bk->name = ss[1].substr(0, eps - ss[1].begin());
			}
			else bk->name = ss[1];

			if (ira) {
				bk->dimVals.resize(lnsz - 1);
				bk->dimNames.insert(bk->dimNames.end(), lns.begin() + 1, lns.end());
				bk->stride = AnScript::StrideOf(bk->typeName[0]);
				if (!bk->stride) {
					_ER("CReader", "unsupported type \"" + bk->typeName + "\" for list!");
					return false;
				}
				bk->typeName = "list(" + std::to_string(lnsz-1) + bk->typeName[0] + ")";
			}
			cv.push_back(std::pair<string, string>(bk->name, bk->typeName));

			if (AnWeb::hasC) {
				if (!(bk->value = scr->lib->GetSym(bk->name))) {
					_ER("CReader", "cannot find \"" + bk->name + "\" from memory!");
					return false;
				}
				if (ira) {
					for (size_t a = 0; a < bk->dimVals.size(); a++) {
						int es = TryParse(bk->dimNames[a], 0);
						if (es > 0) {
							bk->data.dims.resize(a + 1);
							bk->data.dims[a] = es;
							bk->dimVals[a] = &bk->data.dims[a];
						}
						else if (!(bk->dimVals[a] = (int*)scr->lib->GetSym(bk->dimNames[a]))) {
							_ER("CReader", "cannot find \"" + bk->dimNames[a] + "\" from memory!");
							return false;
						}
					}
				}
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
	if (s == "int") var->type = AN_VARTYPE::INT;
	else if (s == "double") var->type = AN_VARTYPE::DOUBLE;
	else return false;
	return true;
}