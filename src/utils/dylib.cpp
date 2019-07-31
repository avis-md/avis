// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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
	CheckUniqueRef();
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

bool DyLib::ForceUnload(DyLib& lib, std::string path) {
	if (!lib.lib) return true;
#ifdef PLATFORM_WIN
	return FreeLibrary((HMODULE)lib.lib);
#else
	for (int tries = 0; tries < 10; ++tries) {
		dlclose(lib.lib);
		sleep(10);
		if (!dlopen(path.c_str(), RTLD_LAZY | RTLD_NOLOAD))
			return true;
	}
	Debug::Warning("DyLib", "failed to force unloading of \"" + path + "\"!");
	return false;
#endif
}