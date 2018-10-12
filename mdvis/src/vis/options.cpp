#include "options.h"
#include "system.h"

void Options::Init() {
	tab* t;
	option* o;
#define NT(nm) tabs.push_back(tab()); t = &tabs.back(); t->name = #nm
#define NO(nm, key) t->options.push_back(option()); o = &t->options.back(); o->name = #nm; o->_value = &VisSystem::key; o->value = *o->_value
	NT(Environment);
	NO(Python path, envs["PYENV"]);
	NO(MinGW path, envs["MINGW"]);
	NO(VCVARS path, envs["VCBAT"]);
}