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

#pragma once
#include "web/anweb.h"

class CReader {
public:
	static void Init();
	static void LoadReader();

	static bool Read(CScript* scr);
	static void Refresh(CScript* scr);

	friend class CScript;
	friend class FReader;
protected:
	static std::string gpp;
	static std::string vcbatPath, mingwPath;
	static bool useMsvc, useOMP, useOMP2;
	static std::string flags1, flags2;
	static AN_VARTYPE ParseType(const std::string& s);

	struct VarInfo {
		std::string name;
		AN_VARTYPE type, itemType;
		std::string error;
	};
	static VarInfo ParseVar(std::string s);
};