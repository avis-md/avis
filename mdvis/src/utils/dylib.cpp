#include "dylib.h"

DyLib::DyLib(string s) {
#ifdef PLATFORM_WIN
	lib = LoadLibrary(&s[0]);
#else
	lib = dlopen(s);
#endif
}

void* DyLib::GetSym(string s) {
#ifdef PLATFORM_WIN
	return GetProcAddress((HMODULE)lib, &s[0]);
#else
	return dlsym(lib, s);
#endif
}