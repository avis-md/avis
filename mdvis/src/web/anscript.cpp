#include "anweb.h"

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