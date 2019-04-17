#include "anweb.h"

void AnScript::AddInput(std::string nm, std::string tp) {
	invars.push_back(std::pair<std::string, std::string>(nm, tp));
	invaropts.push_back(VarOpt());
}

void AnScript::AddInput(const CVar& cv) {
	AddInput(cv.name, cv.typeName);
}

void AnScript::AddOutput(std::string nm, std::string tp) {
	outvars.push_back(std::pair<std::string, std::string>(nm, tp));
}

void AnScript::AddOutput(const CVar& cv) {
	AddOutput(cv.name, cv.typeName);
}

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