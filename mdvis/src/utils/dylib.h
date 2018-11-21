#pragma once
#include "Engine.h"

class DyLib : public RefCnt {
public:
	DyLib();
	DyLib(std::string nm);
	~DyLib();

	void* GetSym(std::string nm);
	bool is_open();
	void DestroyRef() override { Unload(); }
	void Unload();

	static bool ForceUnload(DyLib* lib, std::string nm);
private:
	void* lib;
};