#ifndef PLATFORM_WIN
#include <dlfcn.h>
#endif
#include "dylib.h"

DyLib::DyLib(string s) {
#ifdef PLATFORM_WIN
	lib = LoadLibrary(&s[0]);
#else
	lib = dlopen(&s[0], RTLD_LAZY);
#endif
}

DyLib::~DyLib() {
#ifdef PLATFORM_WIN
	FreeLibrary((HMODULE)lib);
#else
	dlclose(lib);
#endif
}

void DyLib::ForceUnload(DyLib* lib, string path) {
#ifndef PLATFORM_WIN
	for (;;) {
		dlclose(lib->lib);
		if (!dlopen(path.c_str(), RTLD_LAZY | RTLD_NOLOAD))
			break;
	}
#endif
}

void* DyLib::GetSym(string s) {
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