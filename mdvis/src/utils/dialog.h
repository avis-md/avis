#pragma once
#include "Engine.h"

class Dialog {
public:
	static std::vector<string> OpenFile(std::vector<string> pattern, bool multi = false);
};