#pragma once
#include "Engine.h"

class DyLib {
public:
	DyLib(string nm);
	~DyLib();

	void* GetSym(string nm);
	bool is_open();
	
private:
	void* lib;
};