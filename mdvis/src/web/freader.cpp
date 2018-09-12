#include "anweb.h"
#include "vis/system.h"
#ifndef IS_ANSERVER
#include "utils/runcmd.h"
#endif

//#define _GEN_DEBUG

void FReader::Init() {
#ifdef PLATFORM_WIN
	if (IO::HasFile(CReader::mingwPath + "/gfortran.exe") && IO::HasFile(CReader::mingwPath + "/g++.exe")) {
		AnWeb::hasFt = true;
	}
#else
	AnWeb::hasFt = true;
#endif
}

bool FReader::Read(FScript* scr) {
	string path = scr->path;
	string fp = IO::path + "/nodes/" + path;
	std::replace(fp.begin(), fp.end(), '\\', '/');
	auto s = IO::GetText(fp);
	fp = fp.substr(0, fp.size() - 4);
	auto ls = fp.find_last_of('/');
	string& nm = scr->name = fp.substr(ls + 1);
	string fp2 = fp.substr(0, ls + 1) + "__fcache__/";


#ifndef IS_ANSERVER
	if (!IO::HasDirectory(fp2)) IO::MakeDirectory(fp2);

	if (AnWeb::hasFt) {
		auto mt = scr->chgtime = IO::ModTime(fp + ".f90");
		if (mt < 0) return false;
		auto ot = IO::ModTime(fp2 + nm + ".so");

		bool fail = false;
	#define _ER(a, b)\
				scr->compileLog.push_back(ErrorView::Message());\
				auto& msgg = scr->compileLog.back();\
				msgg.name = path;\
				msgg.path = fp + ".cpp";\
				msgg.msg.resize(1, "(System) " b);\
				msgg.severe = true;\
				scr->errorCount = 1;\
				Debug::Warning(a "(" + scr->name + ")", b);

		string funcNm = "";

		if (mt >= ot) {
			string lp(fp2 + nm + ".so");
			remove(&lp[0]);

			{
				std::ifstream strm(fp + ".f90");
				std::ofstream ostrm(fp + "_temp__.f90");
				std::vector<string> arr_o;
				string s;
				int tp = 0;
				size_t loc = -1;
				while (!fail && std::getline(strm, s)) {
					if (s[0] != '!') {
						switch (tp) {
						case 0:
							ostrm << s << "\n";
							break;
						case 1:
						case 2:
							loc = s.find_first_of(':');
							if (loc < 2) {
								_ER("FReader", "Variable parse error after \"!in\"!");
								break;
							}
							if (string_find(to_lowercase(s), ", allocatable") > -1) {
								ostrm << s << "\n";
								if (tp == 2) {
									auto s2 = rm_spaces(s);
									auto l1 = string_find(s2, "::");
									auto l2 = string_find(s2, "(:");
									if (l1 == -1 || l2 == -1) {
										_ER("FReader", "Failed to parse array name!");
										break;
									}
									arr_o.push_back(s2.substr(l1 + 2, l2 - l1 - 2));
								}
							}
							else
								ostrm << s.substr(0, loc - 1) << ", BIND(C) " << s.substr(loc) << "\n";
							break;
						case 3:
							auto sl = string_find(s, "subroutine");
							if (sl == -1) {
								_ER("FReader", "Subroutine missing after \"!entry\"!");
								break;
							}
							auto p1 = s.find_first_not_of(' ', sl + 11);
							auto bl = string_find(s, "()", p1 + 1);
							if (sl == -1) {
								_ER("FReader", "\"()\" missing after \"!entry\"!");
								break;
							}
							funcNm = s.substr(p1, bl - p1);
							ostrm << s << " BIND(C)\n";
							std::ofstream ets(fp2 + nm + ".entry");
							ets << to_lowercase(funcNm);
							break;
						}
						tp = 0;
					}
					else {
						if (s == "!in") tp = 1;
						if (s == "!out") tp = 2;
						if (s == "!entry") tp = 3;
						ostrm << "\n";
					}
				}

				GenArrIO(fp2, nm, arr_o);
				ostrm << "\ninclude \"__fcache__/" + nm + ".ext.f95\"";
			}

			if (fail) {
				remove((fp + "_temp__.cpp").c_str());
				return false;
			}



			string cmd = CReader::gpp + " -shared "
			#ifdef PLATFORM_WIN
				"-static-libstdc++ -static-libgcc -Wl,--export-all-symbols "
			#else
				"-lc++ "
			#endif
			"-fPIC \"" + IO::path + "/res/noterminate.o\" -o \""
				+ fp2 + nm + ".so\" \"" + fp + "_temp__.f90\" -lgfortran 2> \"" + fp2 + nm + "_log.txt\"";
			std::cout << cmd << std::endl;
			#ifdef PLATFORM_WIN
				RunCmd::Run("path=%path%;" + CReader::mingwPath + " && " + cmd);
			#else
				RunCmd::Run(cmd);
			#endif
			//scr->errorCount = ErrorView::Parse_GCC(fp2 + nm + "_log.txt", fp + "_temp__.cpp", nm + ".cpp", scr->compileLog);
			for (auto& m : scr->compileLog) {
				m.path = fp + ".f90";
			}

			remove((fp + "_temp__.f90").c_str());
		}

#endif
		scr->libpath = fp2 + nm + ".so";
		scr->lib = new DyLib(scr->libpath);
		if (!scr->lib->is_open()) {
			_ER("FReader", "Failed to load script into memory!");
			return false;
		}
		{
			std::ifstream ets(fp2 + nm + ".entry");
			ets >> funcNm;
		}
		auto acf = (emptyFunc)scr->lib->GetSym(funcNm);
		if (!acf) {
			string err = 
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			_ER("FReader", "Failed to load function \"" + funcNm + "\" into memory! " + err);
			return false;
		}
		auto fhlc = (emptyFunc*)scr->lib->GetSym("__noterm_ftFunc");
		if (!fhlc) {
			_ER("FReader", "Failed to register function pointer! Please tell the monkey!");
			return false;
		}
		*fhlc = acf;
		
		scr->funcLoc = (wrapFunc)scr->lib->GetSym("_Z5ExecFv");
		if (!scr->funcLoc) {
			_ER("FReader", "Failed to register entry function! Please tell the monkey!");
			return false;
		}

		scr->arr_shapeloc = (int32_t**)scr->lib->GetSym("__mod_exp_MOD_exp_arr_shp");
		if (!scr->arr_shapeloc) {
			_ER("FReader", "Failed to register array shape pointer! Please tell the monkey!");
			return false;
		}
		scr->arr_dataloc = (void**)scr->lib->GetSym("exp_arr_ptr");
		if (!scr->arr_dataloc) {
			_ER("FReader", "Failed to register array data pointer! Please tell the monkey!");
			return false;
		}
	}


	std::ifstream strm(fp + ".f90");
	string ln;
	bool fst = true;
	while (!strm.eof()) {
		std::getline(strm, ln);
		while (fst && ln[0] == '!' && ln[1] == '!' && ln[2] == ' ') {
			scr->desc += ln.substr(3) + "\n";
			scr->descLines++;
			std::getline(strm, ln);
		}
		fst = false;

		bool iso = ln == "!out";
		if (iso || ln == "!in") {
			std::vector<std::pair<string, string>>& cv = iso ? scr->outvars : scr->invars;
			std::vector<CVar>& _cv = iso ? scr->_outvars : scr->_invars;
			std::vector<emptyFunc>& _fc = iso? scr->_outarr_post : scr->_inarr_pre;
			_cv.push_back(CVar());
			auto bk = &_cv.back();
			_fc.push_back(emptyFunc());
			auto fk = &_fc.back();

			const string ios = iso ? "output " : "input ";

			std::getline(strm, ln);
			auto ss = string_split(ln, ' ', true);
			auto sss = ss.size();
			int atn = 0;
			while (ss[atn].back() == ',') {
				ss[atn].pop_back();
				atn++;
			}
			bk->typeName = ss[0];
			if (!ParseType(bk->typeName, bk)) {
				_ER("FReader", "arg type \"" + bk->typeName + "\" not recognized!");
				return false;
			}
			bk->stride = AnScript::StrideOf(bk->typeName[0]);
			if (sss < atn + 2) {
				_ER("FReader", "variable name not found!");
				return false;
			}
			bk->name = ss[atn + 2];
			bool isa = false;
			if (sss > atn + 3) {
				auto sa = ss[atn + 3];
				auto sas = sa.size();
				if (sa.substr(0, 2) == "(:" && sa.substr(sas-2) == ":)") {
					bk->dimVals.push_back(new int());
					for (auto c : sa) {
						if (c == ',') bk->dimVals.push_back(new int());
					}
					isa = true;
				}
			}
			
			if (isa) {
				bk->type = AN_VARTYPE::LIST;
				bk->typeName = "list(" + std::to_string(bk->dimVals.size()) + bk->typeName[0] + ")";
			}
			cv.push_back(std::pair<string, string>(bk->name, bk->typeName));
			
			if (AnWeb::hasFt) {
				auto nml = to_lowercase(bk->name);
				if (!isa)
					bk->value = scr->lib->GetSym(nml);
				else {
					bk->value = scr->lib->GetSym("__test_MOD_" + nml);
					if (iso) *fk = (emptyFunc)scr->lib->GetSym("exp_get_" + nml);
				}
				if (!bk->value) {
					Debug::Warning("FReader", "cannot find \"" + bk->name + "\" from memory!");
					return false;
				}
			}
		}
	}


	FScript::allScrs.emplace(path, scr);

	return true;
}

void FReader::Refresh(FScript* scr) {
	auto mt = IO::ModTime(IO::path + "/nodes/" + scr->path);
	if (mt > scr->chgtime) {
		Debug::Message("FReader", "Reloading " + scr->path);
		scr->Clear();
		scr->ok = Read(scr);
	}
}

bool FReader::ParseType(string& s, CVar* var) {
	s = to_lowercase(s);
	if (s == "integer") {
		var->type = AN_VARTYPE::INT;
		s = "int";
	}
	else if (s == "real*8") {
		var->type = AN_VARTYPE::DOUBLE;
		s = "double";
	}
	else return false;
	return true;
}

void FReader::GenArrIO(string path, string name, std::vector<string> outvars) {
	std::ofstream strm(path + name + ".ext.f95");

	strm << R"(module mod_exp
 use iso_c_binding
 use )" << name << R"(
 implicit none
    integer, allocatable, target :: exp_arr_shp(:)
    type(c_ptr), bind(c) :: exp_arr_ptr

 contains 
)";
	for (auto& v : outvars) {
		strm << "    subroutine exp_get_" + v + "() bind(c)\n\
        exp_arr_shp = shape(" + v + ")\n\
        exp_arr_ptr = c_loc(" + v + ")\n\
    end subroutine exp_get_" + v + "\n";
	}
	strm << "end module mod_exp";
}