#include "anweb.h"

bool AnScript::Clear() {
	inputs.clear();
	inputs.clear();
	desc = "";
	descLines = 0;
	ok = false;
	return true;
}

void* AnScript_I::Resolve(uintptr_t o) {
	return (void*)((uintptr_t)instance + o);
}

void AnScript_I::Execute() {
	parent->caller(instance);
}