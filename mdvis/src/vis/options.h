#pragma once
#include "Engine.h"

class Options {
public:
	struct option {
		std::string name;
		std::string value, *_value;
	};
	struct tab {
		std::string name;
		std::vector<option> options;
	};
	
	std::vector<tab> tabs;

	void Init();
};