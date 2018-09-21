#pragma once
#include "Engine.h"

class DyLib {
public:
	DyLib(std::string nm);
	~DyLib();
	static void ForceUnload(DyLib* lib, std::string nm);

	void* GetSym(std::string nm);
	bool is_open();

private:
	void* lib;
};