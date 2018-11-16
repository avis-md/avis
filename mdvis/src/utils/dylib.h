#pragma once
#include "Engine.h"

class DyLib : public RefCnt<DyLib> {
public:
	DyLib();
	DyLib(std::string nm);
	~DyLib();

	void* GetSym(std::string nm);
	bool is_open();
	void Unload();

	static bool ForceUnload(DyLib* lib, std::string nm);
private:
	void* lib;
};