#pragma once
#include "Engine.h"

class HelpMenu {
public:

	static bool show;
	static float alpha;

	static void Draw();
	static void Link(float x, float y, const std::string& path);
};