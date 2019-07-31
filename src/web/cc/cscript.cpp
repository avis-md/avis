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

#include "web/anweb.h"

std::unordered_map<std::string, std::weak_ptr<CScript>> CScript::allScrs;

CScript::CScript() : AnScript(TYPE::C), progress(0), stdioClr(nullptr),
	stdioLock(nullptr), stdioPtr(nullptr), stdioCnt(nullptr), stdioI(0) {}

void CScript::Clear() {
	AnScript::Clear();
	_inputs.clear();
	_outputs.clear();
}

pAnScript_I CScript::CreateInstance() {
	auto res = std::make_shared<CScript_I>();
	res->Init(this);
	res->instance = isSingleton ? nullptr : spawner();
	return res;
}

void CScript::RegInstances() {
	if (!isSingleton) {
		for (auto i : instances) {
			i->instance = spawner();
		}
	}
	else {
		for (auto i : instances) {
			i->instance = nullptr;
		}
	}
}

void CScript::UnregInstances() {
	for (auto i : instances) {
		if (i->instance) deleter(i->instance);
	}
}

CScript_I::~CScript_I() {
	if (!parent->isSingleton) parent->deleter(instance);
}

void* CScript_I::Resolve(uintptr_t o) {
	return parent->isSingleton ? (void*)o : (void*)((uintptr_t)instance + o);
}

int* CScript_I::GetDimValue(const CVar::szItem& i) {
	return i.useOffset ? (int*)Resolve(i.offset) : (int*)&i.size;
}

#define CS_SET(t) void CScript_I::SetInput(int i, t val) {\
	auto scr = ((CScript*)parent);\
	*(t*)Resolve(scr->_inputs[i].offset) = val;\
}

CS_SET(short)

CS_SET(int)

CS_SET(double)

void CScript_I::SetInput(int i, void* val, char tp, std::vector<int> szs) {
	auto& iv = ((CScript*)parent)->_inputs[i];
	auto& ofs = iv.szOffsets;
	for (int a = 0; a < szs.size(); ++a) {
		*GetDimValue(ofs[a]) = szs[a];
	}
	*(void**)Resolve(iv.offset) = val;
}

void CScript_I::GetOutput(int i, int* val) {
	auto scr = ((CScript*)parent);
	*val = *(int*)Resolve(scr->_inputs[i].offset);
}

float CScript_I::GetProgress() {
	auto scr = ((CScript*)parent);
	if (!scr->progress) return AnScript_I::GetProgress();
	else return Clamp((float)(*(double*)Resolve(scr->progress)), 0.f, 1.f);
}