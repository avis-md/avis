#pragma once
#include "Engine.h"

class Dialog {
public:
	static std::vector<std::string> OpenFile(std::vector<std::string> pattern, bool multi = false);
	static std::string SaveFile(std::string ext);
};