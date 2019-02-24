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