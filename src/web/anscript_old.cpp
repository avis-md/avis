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

#include "anweb.h"

void AnScript::AddInput(std::string nm, std::string tp) {
	invars.push_back(std::pair<std::string, std::string>(nm, tp));
	invaropts.push_back(VarOpt());
}

void AnScript::AddInput(const CVar& cv) {
	AddInput(cv.name, cv.typeName);
}

void AnScript::AddOutput(std::string nm, std::string tp) {
	outvars.push_back(std::pair<std::string, std::string>(nm, tp));
}

void AnScript::AddOutput(const CVar& cv) {
	AddOutput(cv.name, cv.typeName);
}

bool AnScript::Clear() {
	invars.clear();
	outvars.clear();
	desc = "";
	descLines = 0;
	ok = false;
	return true;
}

int AnScript::StrideOf(char c) {
	switch (c) {
	case 's':
		return 2;
	case 'i':
		return 4;
	case 'd':
		return 8;
	default:
		return 0;
	}
}