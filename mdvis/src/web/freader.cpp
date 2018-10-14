#include "anweb.h"
#include "vis/system.h"
#ifndef IS_ANSERVER
#include "utils/runcmd.h"
#endif

#ifdef PLATFORM_WIN
#define SETPATH "path=%path%;" + CReader::mingwPath + "&&" + 
#else
#define SETPATH
#endif

//#define _GEN_DEBUG

void FReader::Init() {
#ifdef PLATFORM_WIN
	if (IO::HasFile(CReader::mingwPath + "/gfortran.exe") && IO::HasFile(CReader::mingwPath + "/g++.exe")) {
		AnWeb::hasFt = true;
	}
#else
	if (AnWeb::hasC)
		AnWeb::hasFt = true;
	else
		Debug::Warning("CReader", "Fortran compiler depends on C++ compiler!");
#endif
}

bool FReader::Read(FScript* scr) {
	std::string& path = scr->path;
	std::string fp = IO::path + "nodes/" + path;
	auto s = IO::GetText(fp);
	auto ls = fp.find_last_of('/');
	std::string& nm = scr->name;
	std::string fp2 = fp.substr(0, ls + 1) + "__fcache__/";


#ifndef IS_ANSERVER
	if (!IO::HasDirectory(fp2)) IO::MakeDirectory(fp2);

	if (AnWeb::hasFt) {
		auto mt = scr->chgtime = IO::ModTime(fp + EXT_FS);
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

		std::string funcNm = "";

		if (mt >= ot) {
			std::string lp(fp2 + nm + ".so");
			remove(&lp[0]);

			{
				std::ifstream strm(fp + EXT_FS);
				std::ofstream ostrm(fp + "_temp__" EXT_FS);
				std::vector<typestring> arr_i;
				std::vector<std::string> arr_o;
				std::string s;
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
								auto s2 = rm_spaces(s);
								auto l1 = string_find(s2, "::");
								auto l2 = string_find(s2, "(:");
								if (l1 == -1 || l2 == -1) {
									_ER("FReader", "Failed to parse array name!");
									break;
								}
								auto nm = s2.substr(l1 + 2, l2 - l1 - 2);
								if (tp == 1) {
									auto tn = s2.substr(0, s2.find_first_of(','));
									arr_i.push_back(typestring(tn, nm, s2.substr(l2)));
								}
								arr_o.push_back(nm);
							}
							else
								ostrm << s.substr(0, loc - 1) << ", BIND(C) " << s.substr(loc) << "\n";
							break;
						case 3:
							auto sl = string_find(to_lowercase(s), "subroutine");
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

				GenArrIO(fp2, nm, arr_i, arr_o);
				ostrm << "\ninclude \"__fcache__/" + nm + ".ext.f95\"";
			}

			if (fail) {
				remove((fp + "_temp__.cpp").c_str());
				return false;
			}



			std::string cmd = CReader::gpp + " -shared "
			#ifdef PLATFORM_WIN
				"-static-libstdc++ -static-libgcc -Wl,--export-all-symbols "
			#else
				"-D/tmp/ "
			#endif
			"-fPIC \"" + IO::path + "res/noterminate.o\" -J \"" + fp2 + "\" -o \""
				+ fp2 + nm + ".so\" \"" + fp + "_temp__" EXT_FS "\" -lgfortran 2> \"" + fp2 + nm + "_log.txt\"";
			RunCmd::Run(SETPATH cmd);
			scr->errorCount = ErrorView::Parse_GFortran(fp2 + nm + "_log.txt", fp + "_temp__" EXT_FS, nm + EXT_FS, scr->compileLog);
			for (auto& m : scr->compileLog) {
				m.path = fp + EXT_FS;
			}

			remove((fp + "_temp__" EXT_FS).c_str());
		}

#endif

		Engine::AcquireLock(7);
#define FAIL0 Engine::ReleaseLock(); return false
		scr->libpath = fp2 + nm + ".so";
		scr->lib = new DyLib(scr->libpath);
		if (!scr->lib->is_open()) {
			_ER("FReader", "Failed to load script into memory!");
			FAIL0;
		}
		{
			std::ifstream ets(fp2 + nm + ".entry");
			ets >> funcNm;
		}
		auto acf = (emptyFunc)scr->lib->GetSym(funcNm);
		if (!acf) {
			std::string err = 
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			_ER("FReader", "Failed to load function \"" + funcNm + "\" into memory! " + err);
			FAIL0;
		}
		auto fhlc = (emptyFunc*)scr->lib->GetSym("__noterm_ftFunc");
		if (!fhlc) {
			_ER("FReader", "Failed to register function pointer! Please tell the monkey!");
			FAIL0;
		}
		*fhlc = acf;

#define _ER2(info) _ER("FReader", "Failed to register " info "! Please tell the monkey!");
		
		scr->funcLoc = (wrapFunc)scr->lib->GetSym("_Z5ExecFv");
		if (!scr->funcLoc) {
			_ER2("entry function");
			FAIL0;
		}

		scr->arr_in_shapeloc = (int32_t**)scr->lib->GetSym("imp_arr_shp");
		if (!scr->arr_in_shapeloc) {
			_ER2("input array shape pointer");
			FAIL0;
		}
		scr->arr_in_dataloc = (void**)scr->lib->GetSym("imp_arr_ptr");
		if (!scr->arr_in_dataloc) {
			_ER2("input array data pointer");
			FAIL0;
		}

		scr->arr_out_shapeloc = (int32_t**)scr->lib->GetSym("__mod_exp_MOD_exp_arr_shp");
		if (!scr->arr_out_shapeloc) {
			_ER2("output array shape pointer");
			FAIL0;
		}
		scr->arr_out_dataloc = (void**)scr->lib->GetSym("exp_arr_ptr");
		if (!scr->arr_out_dataloc) {
			_ER2("output array data pointer");
			FAIL0;
		}
	}
	else Engine::AcquireLock(7);

	std::ifstream strm(fp + EXT_FS);
	std::string ln;
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
			std::vector<std::pair<std::string, std::string>>& cv = iso ? scr->outvars : scr->invars;
			std::vector<CVar>& _cv = iso ? scr->_outvars : scr->_invars;
			std::vector<emptyFunc>& _fc = iso? scr->_outarr_post : scr->_inarr_pre;
			_cv.push_back(CVar());
			auto bk = &_cv.back();
			_fc.push_back(emptyFunc());
			auto fk = &_fc.back();

			const std::string ios = iso ? "output " : "input ";

			std::getline(strm, ln);
			auto ss = string_split(ln, ' ', true);
			auto sss = ss.size();
			size_t atn = 0;
			while (ss[atn].back() == ',') {
				ss[atn].pop_back();
				atn++;
			}
			bk->typeName = ss[0];
			if (!ParseType(bk->typeName, bk)) {
				_ER("FReader", "arg type \"" + bk->typeName + "\" not recognized!");
				goto FAIL;
			}
			bk->stride = AnScript::StrideOf(bk->typeName[0]);
			if (sss < atn + 2) {
				_ER("FReader", "variable name not found!");
				goto FAIL;
			}
			bk->name = to_lowercase(ss[atn + 2]);
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
			cv.push_back(std::pair<std::string, std::string>(bk->name, bk->typeName));
			
			if (AnWeb::hasFt) {
				auto nml = to_lowercase(bk->name);
				if (!isa)
					bk->value = scr->lib->GetSym(nml);
				else {
					bk->value = scr->lib->GetSym("__" + to_lowercase(scr->name) + "_MOD_" + nml);
					if (iso) *fk = (emptyFunc)scr->lib->GetSym("exp_get_" + nml);
					else *fk = (emptyFunc)scr->lib->GetSym("imp_set_" + nml);
					if (!fk) {
						_ER2("array convert function")
						goto FAIL;
					}
				}
				if (!bk->value) {
					Debug::Warning("FReader", "cannot find \"" + bk->name + "\" from memory!");
					goto FAIL;
				}
			}
		}
	}

	Engine::ReleaseLock();
	return true;
FAIL:
	Engine::ReleaseLock();
	return false;
}

void FReader::Refresh(FScript* scr) {
	auto mt = IO::ModTime(IO::path + "nodes/" + scr->path + EXT_FS);
	if (mt > scr->chgtime || !scr->ok) {
		AnBrowse::busyMsg = "Reloading " + scr->path + EXT_FS;
		Debug::Message("FReader", AnBrowse::busyMsg);
		scr->Clear();
		scr->ok = Read(scr);
	}
}

bool FReader::ParseType(std::string& s, CVar* var) {
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

void FReader::GenArrIO(std::string path, std::string name, std::vector<typestring> invars, std::vector<std::string> outvars) {
	std::ofstream strm(path + name + ".ext.f95");

	strm << R"(module mod_imp
 use iso_c_binding
 use )" << name << R"(
 implicit none
    type(c_ptr), bind(c) :: imp_arr_shp
    type(c_ptr), bind(c) :: imp_arr_ptr

 contains
)";
	for (auto& v : invars) {
		strm << "    subroutine imp_set_" + v.name + "() bind(c)\n\
        integer, pointer :: imp_p_arr_shp (:)\n\
        " + v.type + ", pointer :: imp_p_arr_ptr " + v.dims << "\n\
        call c_f_pointer(imp_arr_shp, imp_p_arr_shp, [" + std::to_string((v.dims.size() - 1) / 2) + "])\n\
        call c_f_pointer(imp_arr_ptr, imp_p_arr_ptr, imp_p_arr_shp)\n\
        " + v.name + " = imp_p_arr_ptr\n\
    end subroutine imp_set_" + v.name + "\n";
	}
	strm << "end module mod_imp\n\n";

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
