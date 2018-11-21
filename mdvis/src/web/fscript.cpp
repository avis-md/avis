#include "anweb.h"

std::unordered_map<std::string, FScript*> FScript::allScrs;

bool FScript::Clear() {
	AnScript::Clear();
	_invars.clear();
	_outvars.clear();
	if (AnWeb::hasFt) {
		if (!DyLib::ForceUnload(lib, libpath))
			return false;
		lib = DyLib();
	}
	return true;
}

std::string FScript::Exec() {
	auto res = funcLoc();
	if (res)
		throw res;
	return "";
}