#include "anweb.h"
#include "vis/system.h"
#ifndef IS_ANSERVER
#include "utils/runcmd.h"
#endif

#ifdef PLATFORM_WIN
#define SETPATH "path=" + mingwPath + ";%path%&&" + 
#else
#define SETPATH
#endif

std::string CReader::gpp = "g++";
std::string CReader::vcbatPath = "", CReader::mingwPath = "";
bool CReader::useMsvc, CReader::useOMP, CReader::useOMP2;
std::string CReader::flags1, CReader::flags2;

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
	gpp = VisSystem::envs["GPP"];
	auto res = RunCmd::Run("command -v " + gpp + " &> /dev/null");
	if (res == 0)
		AnWeb::hasC = true;
	else
		Debug::Warning("CReader", "C++ compiler \"" + gpp + "\" not available!");
#endif
	useOMP = (VisSystem::prefs["ANL_USE_OPENMP"] == "true");
	useOMP2 = (VisSystem::prefs["ANL_USE_OPENMP_LIB"] == "true");

	if (AnWeb::hasC && !useMsvc && !IO::HasFile(IO::path + "res/noterminate.o")) {
		std::string cmd = gpp + " -std=c++11 -fPIC -c -o \""
			+ IO::path + "res/noterminate.o\" \""
			+ IO::path + "res/noterminate.cpp\"";
		RunCmd::Run(SETPATH cmd);
	}
}

bool CReader::Read(CScript* scr) {
	std::string& path = scr->path;
	std::string fp = IO::path + "nodes/" + path;
	auto ls = fp.find_last_of('/');
	std::string nm = fp.substr(ls + 1);
	std::string fp2 = fp.substr(0, ls + 1) + "__ccache__/";
    
	auto s = IO::GetText(fp + EXT_CS);

#ifndef IS_ANSERVER
	if (!IO::HasDirectory(fp2)) IO::MakeDirectory(fp2);

	if (AnWeb::hasC) {
		auto mt = scr->chgtime = IO::ModTime(fp + EXT_CS);
		if (mt < 0) return false;
		auto ot = IO::ModTime(fp2 + nm + ".so");

		std::string funcNm;

		bool fail = false;
#define _ER(a, b)\
			scr->compileLog.push_back(ErrorView::Message());\
			auto& msgg = scr->compileLog.back();\
			msgg.name = path;\
			msgg.path = fp + EXT_CS;\
			msgg.msg.resize(1, "(System) " b);\
			msgg.severe = true;\
			scr->errorCount = 1;\
			ErrorView::compileMsgs.push_back(msgg);\
			Debug::Warning(a "(" + scr->name + ")", b);

		if (mt > ot || !IO::HasFile(fp2 + nm + ".entry")) {
			remove((fp2 + nm + ".so").c_str());

#ifdef PLATFORM_WIN
			const std::string dlx = " __declspec(dllexport)";
#else
			const std::string dlx = "";
#endif
			{
				std::ifstream strm(fp + EXT_CS);
				std::ofstream ostrm(fp + "_temp__.cpp");
				std::string s;
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
							std::string ib = s.substr(bo + 1, bc - bo - 1);
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
				std::string cl = "cl /nologo /c -Od ";
				if (useOMP) {
					cl += " /openmp";
				}
				cl += " /EHsc /Fo\"" + fp2 + nm + ".obj\" \"" + fp + "_temp__.cpp\"";
				const std::string lk = "link /nologo /dll /out:\"" + fp2 + nm + ".so\" \"" + fp2 + nm + ".obj\"";
				RunCmd::Run("\"" + vcbatPath + "\" && " + cl + " > \"" + fp2 + nm + "_log.txt\" && " + lk + " > \"" + fp2 + nm + "_log.txt\"");
				scr->errorCount = ErrorView::Parse_MSVC(fp2 + nm + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
			}
			else {
				std::string cmd = "g++ -std=c++11 -static-libstdc++ -static-libgcc -shared -fPIC " + flags1;
				if (useOMP) {
					cmd += " -fopenmp";
					if (useOMP2) cmd += " -lomp";
				}
				cmd += " -lm -o \"" + fp2 + nm + ".so\" \"" + IO::path + "res/noterminate.o\" \"" + fp + "_temp__.cpp\" -Wl,--export-all-symbols 2> \"" + fp2 + nm + "_log.txt\"";
				RunCmd::Run(SETPATH cmd);
				scr->errorCount = ErrorView::Parse_GCC(fp2 + nm + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
			}
#else
			std::string cmd = gpp + " -std=c++11 -shared -fPIC "
#ifdef PLATFORM_LNX
				" -fno-gnu-unique "
#endif
				+ flags1;
			if (useOMP) {
#ifdef PLATFORM_OSX
				cmd += " -Xpreprocessor -fopenmp";
#else
				cmd += " -fopenmp";
#endif
				if (useOMP2) cmd += " -lomp";
			}
			cmd += " -lm -o \"" + fp2 + nm + ".so\" \"" + fp + "_temp__.cpp\" 2> " + fp2 + nm + "_log.txt";
			RunCmd::Run(cmd);
			scr->errorCount = ErrorView::Parse_GCC(fp2 + nm + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
#endif
			for (auto& m : scr->compileLog) {
				m.path = fp + EXT_CS;
			}
			ErrorView::compileMsgs.insert(ErrorView::compileMsgs.end(), scr->compileLog.begin(), scr->compileLog.end());
			remove((fp + "_temp__.cpp").c_str());
		}
	
#endif

		scr->libpath = fp2 + nm + ".so";
		scr->lib = new DyLib(scr->libpath);
		if (!scr->lib->is_open()) {
			std::string err =
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
			std::string err =
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			_ER("CReader", "Failed to load function \"" + funcNm + "\" into memory! " + err);
			return false;
		}

#ifdef PLATFORM_WIN
		if (!useMsvc) {
			auto fhlc = (emptyFunc*)scr->lib->GetSym("__noterm_cFunc");
			if (!fhlc) {
				_ER("CReader", "Failed to register function pointer! Please tell the monkey!");
				return false;
			}
			*fhlc = scr->funcLoc;

			scr->wFuncLoc = (wrapFunc)scr->lib->GetSym("_Z5ExecCv");
			if (!scr->funcLoc) {
				_ER("CReader", "Failed to register entry function! Please tell the monkey!");
				return false;
			}
		}
#endif
	}

	std::ifstream strm(fp + EXT_CS);
	std::string ln;
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
		bool ien = (lnsz > 1)? (lns[1] == "enum") : false; 
		bool irg = (lnsz > 1)? (lns[1] == "range") : false;
		bool iso = (lns[0] == "//out");
		if (lns[0] == "//in" || iso) {
			std::vector<std::pair<std::string, std::string>>& cv = iso ? scr->outvars : scr->invars;
			std::vector<CVar>& _cv = iso ? scr->_outvars : scr->_invars;
			const std::string ios = iso ? "output " : "input ";
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
			else if (ien && (bk->typeName != "int" || ira)) {
				_ER("CReader", "" + ios + " enum must be of int type!");
				return false;
			}
			else if (ien && lnsz < 3) {
				_ER("CReader", "" + ios + " enum has no options!");
				return false;
			}
			else if (irg && lnsz != 4) {
				_ER("CReader", "" + ios + " range must have 2 parameters!");
				return false;
			}
			if (!ira) {
				if (lnsz > 1 && !ien && !irg) {
					_ER("CReader", "" + ios + " with specified size must be of pointer type!");
					return false;
				}
				else if (ien && iso) {
					_ER("CReader", "" + ios + " cannot be an enum!");
					return false;
				}
				else if (irg && iso) {
					_ER("CReader", "" + ios + " cannot be ranged!");
					return false;
				}
			}
			if (!ira && !ParseType(bk->typeName, bk)) {
				std::string add = "";
				if (bk->typeName == "float") add = " Did you mean \"double\"?";
				_ER("CReader", "arg type \"" + bk->typeName + "\" not recognized!" + add);
				return false;
			}
			
			std::string::iterator eps;
			if ((eps = std::find(ss[1].begin(), ss[1].end(), '=')) != ss[1].end()) {
				bk->name = ss[1].substr(0, eps - ss[1].begin());
			}
			else bk->name = ss[1];

			if (ira) {
				bk->dimVals.resize(lnsz - 1);
				bk->dimNames.insert(bk->dimNames.end(), lns.begin() + 1, lns.end());
				bk->type = AN_VARTYPE::LIST;
				bk->stride = AnScript::StrideOf(bk->typeName[0]);
				if (!bk->stride) {
					_ER("CReader", "unsupported type \"" + bk->typeName + "\" for list!");
					return false;
				}
				bk->typeName = "list(" + std::to_string(lnsz-1) + bk->typeName[0] + ")";
			}
			else {
				bk->data.val.d = 0;
			}
			cv.push_back(std::pair<std::string, std::string>(bk->name, bk->typeName));
			if (!iso) {
				scr->invaropts.push_back(VarOpt());
				auto& opt = scr->invaropts.back();
				opt.type = ien? VarOpt::ENUM : (irg? VarOpt::RANGE : VarOpt::NONE);
				if (ien) {
					opt.enums.insert(opt.enums.end(), lns.begin() + 2, lns.end());
					opt.enums.push_back("");
				}
				else if (irg) {
					opt.range = Vec2(TryParse(lns[2], 0.0f), TryParse(lns[3], 1.0f));
				}
			}

			if (AnWeb::hasC) {
				if (!(bk->value = scr->lib->GetSym(bk->name))) {
					_ER("CReader", "cannot find \"" + bk->name + "\" from memory!");
					return false;
				}
				if (ira) {
					auto sz = bk->dimVals.size();
					bk->data.dims.resize(sz);
					for (size_t a = 0; a < sz; a++) {
						int es = TryParse(bk->dimNames[a], 0);
						if (es > 0) {
							if (iso) { //temp fix for mac access error
								bk->data.dims[a] = es;
								bk->dimVals[a] = &bk->data.dims[a];
							}
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

	return true;
}

void CReader::Refresh(CScript* scr) {
	auto mt = IO::ModTime(IO::path + "nodes/" + scr->path + EXT_CS);
	if (mt > scr->chgtime) {
		Debug::Message("CReader", "Reloading " + scr->path + EXT_CS);
		scr->Clear();
		scr->ok = Read(scr);
	}
}

bool CReader::ParseType(std::string s, CVar* var) {
	if (s == "int") var->type = AN_VARTYPE::INT;
	else if (s == "double") var->type = AN_VARTYPE::DOUBLE;
	else return false;
	return true;
}
