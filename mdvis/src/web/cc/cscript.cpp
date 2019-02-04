#include "web/anweb.h"

std::unordered_map<std::string, std::weak_ptr<CScript>> CScript::allScrs;

std::shared_ptr<AnScript_I> CScript::CreateInstance() {
	auto res = std::make_shared<CScript_I>();
	res->parent = this;
	res->instance = spawner();
	return res;
}

#define CS_SET(t) void CScript_I::SetInput(int i, t val) {\
	auto scr = ((CScript*)parent);\
	*(t*)Resolve(scr->_inputs[i].offset) = val;\
}

CS_SET(short)

CS_SET(int)

CS_SET(double)

void CScript_I::SetInput(int i, void* val, char tp, std::vector<int> szs) {
	int sz = 1;
	auto& iv = ((CScript*)parent)->_inputs[i];
	auto& ofs = iv.szOffsets;
	for (int a = 0; a < szs.size(); ++a) {
		*GetDimValue(ofs[a]) = szs[a];
		sz *= szs[a];
	}
	size_t n = 0;
	if (tp == 's') n = 2;
	if (tp == 'i') n = 4;
	if (tp == 'd') n = 8;
	n *= sz;
	if (n > 0)
		std::memcpy(*(void**)Resolve(iv.offset), val, n);
}

void CScript_I::GetOutput(int i, int* val) {
	auto scr = ((CScript*)parent);
	*val = *(int*)Resolve(scr->_inputs[i].offset);
}