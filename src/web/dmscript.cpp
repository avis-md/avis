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

AnScript::Var& DmScript::AddInput(const std::string& name, AN_VARTYPE type, int dim) {
	inputs.push_back(Var());
	auto& vr = inputs.back();
	vr.name = name;
	vr.dim = dim;
	if (dim != 0) {
		vr.type = AN_VARTYPE::LIST;
	}
	auto& tp = (dim != 0) ? vr.itemType : vr.type;
	tp = type;
	vr.InitName();
	vr.uiType = Var::UI_TYPE::NONE;
	return vr;
}

AnScript::Var& DmScript::AddOutput(const std::string& name, AN_VARTYPE type, int dim) {
	outputs.push_back(Var());
	auto& vr = outputs.back();
	vr.name = name;
	vr.dim = dim;
	if (dim != 0) {
		vr.type = AN_VARTYPE::LIST;
	}
	auto& tp = (dim != 0) ? vr.itemType : vr.type;
	tp = type;
	vr.InitName();
	vr.uiType = Var::UI_TYPE::NONE;
	return vr;
}

pAnScript_I DmScript::CreateInstance() {
	auto i = std::make_shared<DmScript_I>();
	i->Init(this);
	return i;
}

void* DmScript_I::Resolve(uintptr_t i) {
	return outputVs[i].pval;
}

int* DmScript_I::GetDimValue(const CVar::szItem& i) {
	return (i.useOffset) ? (int*)i.offset : (int*)&i.size;
}