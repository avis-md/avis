#ifndef PLATFORM_WIN
#include <dlfcn.h>
#endif
#include "dylib.h"

DyLib::DyLib() : lib(nullptr) {}

DyLib::DyLib(std::string s) {
#ifdef PLATFORM_WIN
	lib = LoadLibrary(&s[0]);
#else
	lib = dlopen(&s[0], RTLD_LAZY);
#endif
}

DyLib::~DyLib() {
	if (_IsSingleRef())
		Unload();
}

void* DyLib::GetSym(std::string s) {
#ifdef PLATFORM_WIN
	return GetProcAddress((HMODULE)lib, &s[0]);
#else
#ifdef PLATFORM_OSX
	//s = "_" + s;
#endif
	return dlsym(lib, &s[0]);
#endif
}

bool DyLib::is_open() {
	return lib;
}

void DyLib::Unload() {
	if (lib) {
#ifdef PLATFORM_WIN
		FreeLibrary((HMODULE)lib);
#else
		dlclose(lib);
#endif
		lib = nullptr;
	}
}

bool DyLib::ForceUnload(DyLib* lib, std::string path) {
	if (!lib) return true;
#ifdef PLATFORM_WIN
	return FreeLibrary((HMODULE)lib->lib);
#else
	for (int tries = 0; tries < 10; ++tries) {
		dlclose(lib->lib);
		sleep(10);
		if (!dlopen(path.c_str(), RTLD_LAZY | RTLD_NOLOAD))
			return true;
	}
	Debug::Warning("DyLib", "failed to force unloading of \"" + path + "\"!");
	return false;
#endif
}