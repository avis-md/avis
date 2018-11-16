#include "unloader.h"

Unloader* Unloader::instance = nullptr;

Unloader::Unloader() {
	assert(!instance);
	instance = this;
}

Unloader::~Unloader() {
	Debug::Message("Unloader", "Unloading...");
	for (auto& f : funcs) {
		f();
	}
}

void Unloader::Reg(deinitFunc func) {
	instance->funcs.push_back(func);
}