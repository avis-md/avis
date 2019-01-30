#include "anweb.h"

bool AnScript::Clear() {
	inputs.clear();
	inputs.clear();
	desc = "";
	descLines = 0;
	ok = false;
	return true;
}

void AnScript_I::Execute() {
	parent->caller(instance);
}