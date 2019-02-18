#pragma once
#include <vector>

class Unloader {
public:
	typedef void (*deinitFunc)(void);

	Unloader();
	~Unloader();
	
	static Unloader* instance;

	std::vector<deinitFunc> funcs;

	static void Reg(deinitFunc func);
};