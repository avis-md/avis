#pragma once
#include "Engine.h"

class DyLib {
public:
	DyLib(string nm);
	~DyLib();
	static void ForceUnload(DyLib* lib, string nm);

	void* GetSym(string nm);
	bool is_open();

private:
	void* lib;
};