#include "anweb.h"

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