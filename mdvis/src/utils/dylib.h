#pragma once
#include "Engine.h"

class DyLib {
public:
	DyLib(string nm);

	void* GetSym(string nm);

private:
	void* lib;
};