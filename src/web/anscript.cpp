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

void AnScript::Var::InitName() {
	typeName = AN_VARTYPE_STRS[(int)type];
	if (type == AN_VARTYPE::LIST) {
		typeName[5] = (dim < 0) ? '*' : std::to_string(dim)[0];
		typeName[6] = AN_VARTYPE_STRS[(int)itemType][0];
		stride = AN_VARTYPE_STRIDES[(int)itemType];
	}
	else stride = AN_VARTYPE_STRIDES[(int)type];
}

void AnScript::Clear() {
	inputs.clear();
	outputs.clear();
	desc = "";
	descLines = 0;
	ok = false;
}

AnScript_I::~AnScript_I() {}

void AnScript_I::Init(AnScript* pr) {
	parent = pr;
	defVals.resize(pr->inputs.size());
}

void AnScript_I::Execute() {
	parent->caller(instance);
}

float AnScript_I::GetProgress() {
	return 0.1f;
}