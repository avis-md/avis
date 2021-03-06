// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#include "creader.h"
#include "vis/system.h"
#include "vis/preferences.h"
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
	Preferences::Link("ACL", &flags1);
	Preferences::Link("ALNK", &flags2);
	Preferences::LinkEnv("GPP", &gpp);
#ifdef PLATFORM_WIN
	Preferences::LinkEnv("VCBAT", &vcbatPath);
	Preferences::LinkEnv("MINGW", &mingwPath);
	Preferences::Link("AMSVC", &useMsvc);
#endif
	Preferences::Link("AOMP", &useOMP);
	Preferences::Link("AOMPL", &useOMP2);
}

void CReader::LoadReader() {
#ifdef PLATFORM_WIN
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
	}
#else
	auto res = RunCmd::Run("command -v " + gpp + "> /dev/null 2>&1");
	if (res == 0)
		AnWeb::hasC = true;
	else
		Debug::Warning("CReader", "C++ compiler \"" + gpp + "\" not available!");
#endif
	if (AnWeb::hasC && !useMsvc && !IO::HasFile(IO::path + "res/noterminate.o")) {
		std::string cmd = gpp + " -std=c++11 -fPIC -c -o \""
			+ IO::path + "res/noterminate.o\" \""
			+ IO::path + "res/noterminate.cpp\"";
		RunCmd::Run(SETPATH cmd);
	}
}

bool CReader::Read(CScript* scr) {
	std::string& path = scr->path;
	std::string fp = AnWeb::nodesPath + path;
	auto ls = fp.find_last_of('/');
	const std::string nm = fp.substr(ls + 1);
	const std::string fd = fp.substr(0, ls);
	const std::string fp2 = fp.substr(0, ls + 1) + "__ccache__/";
    
	auto s = IO::GetText(fp + EXT_CS);

#ifndef IS_ANSERVER
	if (!IO::HasDirectory(fp2)) IO::MakeDirectory(fp2);

	auto mt = scr->chgtime = IO::ModTime(fp + EXT_CS);
	if (mt < 0) return false;
	scr->isSingleton = false;

	if (AnWeb::hasC) {
		auto ot = IO::ModTime(fp2 + nm + ".so");

		std::vector<VarInfo> vrNms;
		std::string funcNm, progrsNm;

		bool fail = false;
#define _ER(b)\
			scr->compileLog.push_back(ErrorView::Message());\
			auto& msgg = scr->compileLog.back();\
			msgg.name = path;\
			msgg.path = fp + EXT_CS;\
			msgg.msg.resize(1, "(System) " b);\
			msgg.severe = true;\
			scr->errorCount = 1;\
			ErrorView::compileMsgs.push_back(msgg);\
			Debug::Warning("CReader(" + scr->name + ")", b);

		{
			std::ifstream strm(fp + EXT_CS);
			std::string s;
			const auto classsig = "class" + nm;
			while (std::getline(strm, s)) {
				s = rm_spaces(s);
				if (s.back() == '{') s.pop_back();
				if (s == classsig) {
					scr->isSingleton = false;
					goto foundclass;
				}
			}
			scr->isSingleton = true;
		foundclass:;
		}

		if (mt > ot) {
			remove((fp2 + nm + ".so").c_str());

			#define EXTERN "extern \"C\" __EXPORT__ "
			const std::string tmpPath = fp2 + nm + "_temp__" EXT_CS;
			{
				std::ifstream strm(fp + EXT_CS);
				std::ofstream ostrm(tmpPath);
				std::string s;
				while (std::getline(strm, s)) {
					const auto sc = string_trim(s);
					if (sc[0] == '/' && sc[1] == '/') {
						auto ss = string_split(sc, ' ', true);
						if (ss[0] == "//@in" || ss[0] == "//@out") {
							std::getline(strm, s);
							if (scr->isSingleton) ostrm << EXTERN;
							ostrm << "\n" << s << '\n';
							auto vr = ParseVar(s);
							if (!vr.name[0]) {
								Debug::Warning("CReader", "ParseVar error: " + vr.error);
								fail = true;
								break;
							}
							vrNms.push_back(vr);
						}
						else if (ss[0] == "//@var") {
							std::getline(strm, s);
							if (scr->isSingleton) ostrm << EXTERN;
							ostrm << "\n" << s << '\n';
							auto vr = ParseVar(s);
							if (!vr.name[0]) {
								_ER("ParseVar error: " + vr.error);
								fail = true;
								break;
							}
							else if (vr.type != AN_VARTYPE::INT) {
								_ER("var variable must be of type int!");
								fail = true;
								break;
							}
							vrNms.push_back(vr);
						}
						else if (ss[0] == "//@entry") {
							std::getline(strm, s);
							if (scr->isSingleton) ostrm << EXTERN;
							ostrm << "\n" << s << '\n';
							s = rm_spaces(s);
							auto bo = s.find('(');
							auto bc = s.find(')');
							std::string ib = s.substr(bo + 1, bc - bo - 1);
							if ((*(int32_t*)&s[0] == *(int32_t*)"void") && (ib == "" || ib == "void")) {
								funcNm = s.substr(4, bo - 4);
							}
							else {
								_ER("//@entry must precede a void function with no arguments!");
								fail = true;
								break;
							}
						}
						else if (ss[0] == "//@progress") {
							std::getline(strm, s);
							if (scr->isSingleton) ostrm << EXTERN;
							ostrm << "\n" << s << '\n';
							s = rm_spaces(s);
							auto eq = s.find('=');
							if (s.substr(0, 6) == "double" && eq != std::string::npos) {
								progrsNm = s.substr(6, eq - 6);
							}
							else {
								_ER("//@progress must precede a variable of type double!");
								fail = true;
								break;
							}
						}
						else ostrm << s << "\n";
					}
					else ostrm << s << "\n";
				}

				if (funcNm == "") {
					_ER("Script has no entry point!");
					fail = true;
				}

				ostrm << "\n\n//___magic below___\n\n";
				if (scr->isSingleton) {
					if (progrsNm != "") {
						ostrm << EXTERN "double* __progress"
							" = &" + progrsNm + ";\n\n";
					}
					ostrm << EXTERN "void __func_call(void*) { " + funcNm + "(); }";
				}
				else {
					for (auto& v : vrNms) {
						ostrm << EXTERN "void* __var_" + v.name
							+ " = &((" + nm + "*)nullptr)->" + v.name + ";\n";
					}
					if (progrsNm != "") {
						ostrm << EXTERN "void* __progress"
							" = &((" + nm + "*)nullptr)->" + progrsNm + ";\n";
					}
					ostrm << "\n";
					ostrm << EXTERN + nm + "* __func_spawn() { return new " + nm + "(); }\n"
						EXTERN "void __func_delete(void* t) { delete (" + nm + "*)t; }\n"
						EXTERN "void __func_call(void* t) { ((" + nm + "*)t)->" + funcNm + "(); }";
				}
			}

			if (fail) {
				//remove(tmpPath.c_str());
				return false;
			}

			const auto incfd = "\"" + IO::path + "res/include/\"";
			
#ifdef PLATFORM_WIN
			if (useMsvc) {
				std::string cl = "cl /nologo /c /Zi /Od /I \"" + fd + "\" /I " + incfd +
					" /D_WIN_ /FI _avis_print.h";
				if (useOMP) {
					cl += " /openmp";
				}
				cl += " /EHsc /Fo\"" + fp2 + nm + ".obj\" \"" + tmpPath + "\"";
				const std::string lk = "link /nologo /dll /debug /out:\"" + fp2 + nm + ".so\" \"" + fp2 + nm + ".obj\"";
				RunCmd::Run("\"" + vcbatPath + "\">NUL && " + cl + " > \"" + fp2 + nm + "_log.txt\" && " + lk + " > \"" + fp2 + nm + "_log.txt\"");
				scr->errorCount = ErrorView::Parse_MSVC(fp2 + nm + "_log.txt", tmpPath, nm + ".cpp", scr->compileLog);
			}
			else {
				std::string cmd = "g++ -g -std=c++11 -static-libstdc++ -shared -fPIC -D_WIN_ -I\"" + fd + "\" -I" + incfd + " -include _avis_print.h " + flags1;
				if (useOMP) {
					cmd += " -fopenmp";
					if (useOMP2) cmd += " -lomp";
				}
				cmd += " -lm -o \"" + fp2 + nm + ".so\" \"" + IO::path + "res/noterminate.o\" \"" + tmpPath + "\" -Wl,--export-all-symbols 2> \"" + fp2 + nm + "_log.txt\"";
				RunCmd::Run(SETPATH cmd);
				scr->errorCount = ErrorView::Parse_GCC(fp2 + nm + "_log.txt", tmpPath, nm + ".cpp", scr->compileLog);
			}
#else
			std::string cmd = gpp + " -std=c++11 -shared -fPIC -I\"" + fd + "\" -I\"" + incfd + "\" -D_UNIX_ -include _avis_print.h -fvisibility=hidden "
#ifdef PLATFORM_LNX
				"-fno-gnu-unique "
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
			cmd += " -lm -o \"" + fp2 + nm + ".so\" \"" + tmpPath + "\" 2> " + fp2 + nm + "_log.txt";
			RunCmd::Run(cmd);
			scr->errorCount = ErrorView::Parse_GCC(fp2 + nm + "_log.txt", tmpPath, nm + ".cpp", scr->compileLog);
#endif
			for (auto& m : scr->compileLog) {
				m.path = fp + EXT_CS;
			}
			ErrorView::compileMsgs.insert(ErrorView::compileMsgs.end(), scr->compileLog.begin(), scr->compileLog.end());
			//remove(tmpPath.c_str());
		}
	
#endif
	}

	Engine::Locker _lock(5);

	if (AnWeb::hasC) {
#define FAIL0 Engine::ReleaseLock(); return false
		scr->libpath = fp2 + nm + ".so";
		scr->lib = DyLib(scr->libpath);
		if (!scr->lib.is_open()) {
			std::string err =
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			_ER("Failed to load script into memory! " + err);
			FAIL0;
		}

#define CHKPTR(vr) if (!scr->vr) {\
				_ER("Internal error: failed to register " #vr " from compiled binary!");\
				FAIL0;\
			}

			scr->caller = (AnScript::callerFunc)scr->lib.GetSym("__func_call");
			auto prgs = scr->lib.GetSym("__progress");
			if (prgs) scr->progress = *(uintptr_t*)prgs;
			scr->stdioClr = (CScript::clearFunc)scr->lib.GetSym("__stdio_clear");
			auto pmtx = scr->lib.GetSym("__stdio_plock");
			if (!pmtx) {\
				_ER("Internal error: failed to register *stdioLock from compiled binary!");\
				FAIL0;\
			}
			scr->stdioLock = *(std::mutex**)pmtx;
			scr->stdioPtr = (void***)scr->lib.GetSym("__stdio_data");
			scr->stdioCnt = (int*)scr->lib.GetSym("__stdio_count");

			CHKPTR(caller) CHKPTR(stdioClr) CHKPTR(stdioPtr) CHKPTR(stdioCnt);

		if (!scr->isSingleton) {
			scr->spawner = (AnScript::spawnerFunc)scr->lib.GetSym("__func_spawn");
			scr->deleter = (AnScript::deleterFunc)scr->lib.GetSym("__func_delete");


			CHKPTR(spawner) CHKPTR(deleter)
		}
	}
	else {
		scr->chgtime = scr->badtime;
	}

	std::ifstream strm(fp + EXT_CS);
	std::string ln;
	bool fst = true;
	while (!strm.eof()) {
		std::getline(strm, ln);
		if (fst && ln == "//@description") {
			std::getline(strm, ln);
			while (ln[0] == '/' && ln[1] == '/' && ln[2] == '@') {
				scr->desc += ln.substr(3) + "\n";
				scr->descLines++;
				std::getline(strm, ln);
			}
			fst = false;
		}

		auto lns = string_split(string_trim(ln), ' ', true);
		auto lnsz = lns.size();
		if (!lnsz) continue;
		bool ien = (lnsz > 1)? (lns[1] == "enum") : false;
		bool irg = (lnsz > 1)? (lns[1] == "range") : false;
		bool iso = (lns[0] == "//@out");
		if (lns[0] == "//@in" || iso) {
			const std::string ios = iso ? "output " : "input ";
			
			auto& vars = iso? scr->outputs : scr->inputs;
			vars.push_back(AnScript::Var());
			auto& vr = vars.back();

			auto& cvars = iso? scr->_outputs : scr->_inputs;
			cvars.push_back(CVar());
			auto& cvr = cvars.back();
			
			std::getline(strm, ln);
			bool ira = false;

			auto ss = string_split(string_trim(ln), ' ', true);
			vr.typeName = ss[0];
			if (vr.typeName.back() == '*') {
				vr.typeName.pop_back();
				ira = true;
			}
			else if (ss[1][0] == '*') {
				ss[1] = ss[1].substr(1);
				ira = true;
			}

			if (ira && lnsz == 1) {
				_ER("Plain " + ios + " must be of non-pointer type!");
				return false;
			}
			else if (ien && (vr.typeName != "int" || ira)) {
				_ER("" + ios + " enum must be of int type!");
				return false;
			}
			else if (ien && lnsz < 3) {
				_ER("" + ios + " enum has no options!");
				return false;
			}
			else if (irg && lnsz != 4) {
				_ER("" + ios + " range must have 2 parameters!");
				return false;
			}
			if (!ira) {
				if (lnsz > 1 && !ien && !irg) {
					_ER("" + ios + " with specified size must be of pointer type!");
					return false;
				}
				else if (ien && iso) {
					_ER("" + ios + " cannot be an enum!");
					return false;
				}
				else if (irg && iso) {
					_ER("" + ios + " cannot be ranged!");
					return false;
				}
			}
			if (ira) vr.type = AN_VARTYPE::LIST;
			auto& te = ira? vr.itemType : vr.type;
			te = ParseType(vr.typeName);
			if (te == AN_VARTYPE::NONE) {
				std::string add = "";
				if (vr.typeName == "float") add = " Did you mean \"double\"?";
				_ER("variable of type \"" + vr.typeName + "\" not supported!" + add);
				return false;
			}
			if (ira) {
				vr.dim = lnsz - 1;
				vr.typeName = "list(" + std::to_string(vr.dim) + vr.typeName.substr(0, 1) + ")";
			}
			vr.name = ss[1];
			std::string::iterator eps;
			if ((eps = std::find(vr.name.begin(), vr.name.end(), '=')) != vr.name.end()) {
				vr.name = vr.name.substr(0, eps - vr.name.begin());
			}

			/*
			if (ira) {
				bk->dimVals.resize(lnsz - 1);
				bk->dimNames.insert(bk->dimNames.end(), lns.begin() + 1, lns.end());
				bk->type = AN_VARTYPE::LIST;
				bk->stride = AnScript::StrideOf(bk->typeName[0]);
				if (!bk->stride) {
					_ER("unsupported type \"" + bk->typeName + "\" for list!");
					return false;
				}
				bk->typeName = "list(" + std::to_string(lnsz-1) + bk->typeName[0] + ")";
			}
			else {
				bk->data.val.d = 0;
			}
			cv.push_back(std::pair<std::string, std::string>(bk->name, bk->typeName));
			*/
			if (!iso) {
				typedef AnScript::Var::UI_TYPE UI_TYPE;
				vr.uiType = ien? UI_TYPE::ENUM : (irg? UI_TYPE::RANGE : UI_TYPE::NONE);
				if (ien) {
					vr.enums.insert(vr.enums.end(), lns.begin() + 2, lns.end());
					vr.enums.push_back("");
				}
				else if (irg) {
					vr.range = Vec2(TryParse(lns[2], 0.f), TryParse(lns[3], 1.f));
				}
			}

			const auto _vr = (scr->isSingleton ? "" : "__var_");
			if (AnWeb::hasC) {
				auto sym = scr->lib.GetSym(_vr + vr.name);
				if (!sym) {
					_ER("cannot find \"" + vr.name + "\" from memory!");
					return false;
				}
				else
					cvr.offset = scr->isSingleton ? (uintptr_t)sym : *(uintptr_t*)sym;
				if (ira) {
					auto sz = lns.size() - 1;
					cvr.szOffsets.resize(sz);
					for (size_t a = 0; a < sz; ++a) {
						auto& of = cvr.szOffsets[a];
						const auto& ln = lns[a+1];
						uint es = TryParse(ln, 0U);
						if (es > 0) {
							of.useOffset = false;
							of.size = es;
						}
						else {
							of.useOffset = true;
							auto sym = scr->lib.GetSym(_vr + ln);
							if (!sym) {
								_ER("cannot find \"" + ln + "\" from memory!");
								return false;
							}
							else of.offset = scr->isSingleton ? (uintptr_t)sym : *(uintptr_t*)sym;
						}
					}
				}
			}
			vr.name = AnWeb::ConvertName(vr.name);
		}
	}

	return true;
}

void CReader::Refresh(CScript* scr) {
	auto mt = IO::ModTime(AnWeb::nodesPath + scr->path + EXT_CS);
	if ((mt > scr->chgtime) || (!scr->ok && mt > scr->badtime)) {
		AnBrowse::busyMsg = "Reloading " + scr->path + EXT_CS;
		Debug::Message("CReader", AnBrowse::busyMsg);
		if (scr->ok) scr->UnregInstances();
		scr->Clear();
		scr->badtime = mt;
		scr->ok = Read(scr);
		if (scr->ok) scr->RegInstances();
		AnBrowse::changed = true;
	}
}

AN_VARTYPE CReader::ParseType(const std::string& s) {
	if (s == "short") return AN_VARTYPE::SHORT;
	else if (s == "int") return AN_VARTYPE::INT;
	else if (s == "double") return AN_VARTYPE::DOUBLE;
	else return AN_VARTYPE::NONE;
}

#define ER(msg) { info.error = msg; return info; }

CReader::VarInfo CReader::ParseVar(std::string s) {
	CReader::VarInfo info = {};
	s = string_trim(s);
	s = s.substr(0, s.find_first_of('='));
	bool ar = false;
	auto ss = string_split(s, ' ');
	if (ss.size() < 2)
		ER("line is not a variable!")
	auto& s0 = ss[0];
	if (s0 == "static" ||
		s0 == "extern" ||
		s0 == "const")
		ER("keywords are not allowed for variables!")
	auto& s1 = ss[1];
	if (s0.back() == '*') {
		s0.pop_back();
		info.type = AN_VARTYPE::LIST;
	}
	else if (s1[0] == '*') {
		s1 = s1.substr(1);
		info.type = AN_VARTYPE::LIST;
	}
	auto& t = (info.type == AN_VARTYPE::LIST)? info.itemType : info.type;
	if (s0 == "short") t = AN_VARTYPE::SHORT;
	else if (s0 == "int") t = AN_VARTYPE::INT;
	else if (s0 == "double") t = AN_VARTYPE::DOUBLE;
	else ER("unknown variable type: \"" + s0 + "\"!");
	info.name = s1;
	return info;
}