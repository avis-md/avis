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
	res->instance = spawner();
	return res;
}

CScript_I::~CScript_I() {
	parent->deleter(instance);
}

void* CScript_I::Resolve(uintptr_t o) {
	return (void*)((uintptr_t)instance + o);
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