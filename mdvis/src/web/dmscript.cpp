#include "anweb.h"

pAnScript_I DmScript::CreateInstance() {
	auto i = std::make_shared<DmScript_I>();
	i->parent = this;
	return i;
}