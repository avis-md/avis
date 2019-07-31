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

class FReader {
public:
	static void Init();
	static void LoadReader();
	
	static bool Read(FScript* scr);
	static void Refresh(FScript* scr);

protected:
	static bool ParseType(std::string& s, AnScript::Var& var);
	
	struct typestring {
		std::string type, name, dims;

		typestring(std::string a, std::string b, std::string c) : type(a), name(b), dims(c) {}
	};
	static void GenArrIO(std::string path, std::string name, std::vector<typestring> invars, std::vector<std::string> outvars);
};