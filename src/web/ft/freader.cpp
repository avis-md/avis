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

#include "freader.h"
#include "web/cc/creader.h"
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

}

void FReader::LoadReader() {
#ifdef PLATFORM_WIN
	if (!CReader::useMsvc && IO::HasFile(CReader::mingwPath + "/gfortran.exe") && IO::HasFile(CReader::mingwPath + "/g++.exe")) {
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
	std::string fp = AnWeb::nodesPath + path;
	auto s = IO::GetText(fp);
	auto ls = fp.find_last_of('/');
	std::string& nm = scr->name;
	std::string fp2 = fp.substr(0, ls + 1) + "__fcache__/";

	auto mt = scr->chgtime = IO::ModTime(fp + EXT_FS);
	if (mt < 0) return false;

#ifndef IS_ANSERVER
	if (!IO::HasDirectory(fp2)) IO::MakeDirectory(fp2);

	std::string funcNm = "";
	if (AnWeb::hasFt) {
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

		if (mt >= ot) {
			std::string lp(fp2 + nm + ".so");
			remove(&lp[0]);

			const std::string tmpPath = fp2 + nm + "_temp__" EXT_FS;
			{
				std::ifstream strm(fp + EXT_FS);
				std::ofstream ostrm(tmpPath);
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
								_ER("FReader", "Variable parse error after \"" + (tp == 1) ? "!@in\"!" : "!@out\"!");
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
								else arr_o.push_back(nm);
							}
							else
								ostrm << s.substr(0, loc - 1) << ", BIND(C) " << s.substr(loc) << "\n";
							break;
						case 3:
							auto sl = string_find(to_lowercase(s), "subroutine");
							if (sl == -1) {
								_ER("FReader", "Subroutine missing after \"!@entry\"!");
								break;
							}
							auto p1 = s.find_first_not_of(' ', sl + 11);
							auto bl = string_find(s, "()", p1 + 1);
							if (sl == -1) {
								_ER("FReader", "\"()\" missing after \"!@entry\"!");
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
						if (s == "!@in") tp = 1;
						else if (s == "!@out") tp = 2;
						else if (s == "!@entry") tp = 3;
						ostrm << "\n";
					}
				}

				GenArrIO(fp2, nm, arr_i, arr_o);
				ostrm << "\ninclude \"" + nm + ".ext.f95\"";
			}

			if (fail) {
				return false;
			}



			std::string cmd = CReader::gpp + " -shared "
#ifdef PLATFORM_WIN
				"-static-libstdc++ -static-libgcc -Wl,--export-all-symbols "
#else
				"-fvisibility=hidden "
#endif
				"-fPIC \"" + IO::path + "res/noterminate.o\" -I\"" + fp2 + "\" -J\"" + fp2 + "\" -o \""
				+ fp2 + nm + ".so\" \"" + tmpPath + "\" -lgfortran 2> \"" + fp2 + nm + "_log.txt\"";
			RunCmd::Run(SETPATH cmd);
			scr->errorCount = ErrorView::Parse_GFortran(fp2 + nm + "_log.txt", tmpPath, nm + EXT_FS, scr->compileLog);
			for (auto& m : scr->compileLog) {
				m.path = fp + EXT_FS;
			}
		}
	}
#endif

	Engine::Locker locker(7);
	if (AnWeb::hasFt) {
		scr->libpath = fp2 + nm + ".so";
		scr->lib = DyLib(scr->libpath);
		if (!scr->lib.is_open()) {
			_ER("FReader", "Failed to load script into memory!");
			return false;
		}
		{
			std::ifstream ets(fp2 + nm + ".entry");
			ets >> funcNm;
		}
		auto acf = (FScript::emptyFunc)scr->lib.GetSym(funcNm);
		if (!acf) {
			std::string err = 
#ifdef PLATFORM_WIN
				std::to_string(GetLastError());
#else
				"";//dlerror();
#endif
			_ER("FReader", "Failed to load function \"" + funcNm + "\" into memory! " + err);
			return false;
		}

#define _ER2(info) _ER("FReader", "Failed to register " info "! Please tell the monkey!");

		auto fhlc = (FScript::emptyFunc*)scr->lib.GetSym("__noterm_ftFunc");
		if (!fhlc) {
			_ER2("function pointer");
			return false;
		}
		*fhlc = acf;
		
		scr->funcLoc = (FScript::wrapFunc)scr->lib.GetSym("_Z5ExecFv");
		if (!scr->funcLoc) {
			_ER2("entry function");
			return false;
		}

		scr->arr_in_shapeloc = (int32_t**)scr->lib.GetSym("imp_arr_shp");
		if (!scr->arr_in_shapeloc) {
			_ER2("input array shape pointer");
			return false;
		}
		scr->arr_in_dataloc = (void**)scr->lib.GetSym("imp_arr_ptr");
		if (!scr->arr_in_dataloc) {
			_ER2("input array data pointer");
			return false;
		}

		scr->arr_out_shapeloc = (int32_t**)scr->lib.GetSym("__" + nm + "_mod_exp_MOD_exp_arr_shp");
		if (!scr->arr_out_shapeloc) {
			_ER2("output array shape pointer");
			return false;
		}
		scr->arr_out_dataloc = (void**)scr->lib.GetSym("exp_arr_ptr");
		if (!scr->arr_out_dataloc) {
			_ER2("output array data pointer");
			return false;
		}
	}

	std::ifstream strm(fp + EXT_FS);
	std::string ln;
	bool fst = true;
	while (!strm.eof()) {
		std::getline(strm, ln);
		while (fst && ln[0] == '!' && ln[1] == '@') {
			scr->desc += ln.substr(2) + "\n";
			scr->descLines++;
			std::getline(strm, ln);
		}
		fst = false;

		bool iso = ln == "!@out";
		if (iso || ln == "!@in") {
			auto& vr = iso ? scr->outputs : scr->inputs;
			auto& _vr = iso ? scr->_outputs : scr->_inputs;
			std::vector<FScript::emptyFunc>& _fc = iso? scr->_outarr_post : scr->_inarr_pre;
			vr.push_back(AnScript::Var());
			_vr.push_back(CVar());
			auto& bk = vr.back();
			_fc.push_back(FScript::emptyFunc());
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
			int ssp = 0;
			while (ss[0][ssp] < 65) ssp++;
			if (ssp > 0) ss[0] = ss[0].substr(ssp);
			bk.typeName = ss[0];
			if (!ParseType(bk.typeName, bk)) {
				_ER("FReader", "arg type \"" + bk.typeName + "\" not recognized!");
				return false;
			}
			if (sss < atn + 2) {
				_ER("FReader", "variable name not found!");
				return false;
			}
			bk.name = to_lowercase(ss[atn + 2]);
			bool isa = false;
			if (sss > atn + 3) {
				auto sa = ss[atn + 3];
				auto sas = sa.size();
				if (sa.substr(0, 2) == "(:" && sa.substr(sas-2) == ":)") {
					bk.dim++;
					for (auto c : sa) {
						if (c == ',')
							bk.dim++;
					}
					isa = true;
				}
			}
			
			if (isa) {
				bk.itemType = bk.type;
				bk.type = AN_VARTYPE::LIST;
				bk.InitName();
			}
			
			if (AnWeb::hasFt) {
				auto& _bk = _vr.back();
				if (!isa)
					_bk.offset = (uintptr_t)scr->lib.GetSym(bk.name);
				else {
					_bk.offset = (uintptr_t)scr->lib.GetSym("__" + to_lowercase(scr->name) + "_MOD_" + bk.name);
					if (iso) {
						*fk = (FScript::emptyFunc)scr->lib.GetSym("exp_get_" + bk.name);
						_bk.szOffsets.resize(bk.dim);
					}
					else *fk = (FScript::emptyFunc)scr->lib.GetSym("imp_set_" + bk.name);
					if (!fk) {
						_ER2("array convert function");
						return false;
					}
				}
				if (!_bk.offset) {
					Debug::Warning("FReader", "cannot find \"" + bk.name + "\" from memory!");
					return false;
				}
			}

			bk.name = AnWeb::ConvertName(bk.name);
		}
	}
	return true;
}

void FReader::Refresh(FScript* scr) {
	auto mt = IO::ModTime(AnWeb::nodesPath + scr->path + EXT_FS);
	if (mt > scr->chgtime || (!scr->ok && mt > scr->badtime)) {
		AnBrowse::busyMsg = "Reloading " + scr->path + EXT_FS;
		Debug::Message("FReader", AnBrowse::busyMsg);
		scr->Clear();
		scr->badtime = mt;
		scr->ok = Read(scr);
		AnBrowse::changed = true;
	}
}

bool FReader::ParseType(std::string& s, AnScript::Var& var) {
	s = to_lowercase(s);
	if (s == "integer*2") {
		var.type = AN_VARTYPE::SHORT;
		var.stride = 2;
		s = "short";
	}
	else if (s == "integer") {
		var.type = AN_VARTYPE::INT;
		var.stride = 4;
		s = "int";
	}
	else if (s == "real*8") {
		var.type = AN_VARTYPE::DOUBLE;
		var.stride = 8;
		s = "double";
	}
	else return false;
	return true;
}

void FReader::GenArrIO(std::string path, std::string name, std::vector<typestring> invars, std::vector<std::string> outvars) {
	std::ofstream strm(path + name + ".ext.f95");

	strm << "module " + name + R"(_mod_imp
 use iso_c_binding
 use )" << name << R"(
 implicit none
	type(c_ptr), bind(c) :: imp_arr_shp
	type(c_ptr), bind(c) :: imp_arr_ptr

 contains
)";
	for (auto& v : invars) {
		strm << "	subroutine imp_set_" + v.name + "() bind(c)\n\
		integer, pointer :: imp_p_arr_shp (:)\n\
		" + v.type + ", pointer :: imp_p_arr_ptr " + v.dims << "\n\
		call c_f_pointer(imp_arr_shp, imp_p_arr_shp, [" + std::to_string((v.dims.size() - 1) / 2) + "])\n\
		call c_f_pointer(imp_arr_ptr, imp_p_arr_ptr, imp_p_arr_shp)\n\
		" + v.name + " = imp_p_arr_ptr\n\
	end subroutine imp_set_" + v.name + "\n";
	}
	strm << "end module " + name + "_mod_imp\n\n";

	strm << "module " + name + R"(_mod_exp
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
	strm << "end module " + name + "_mod_exp";
}
