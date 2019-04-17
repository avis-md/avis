#pragma once
#include "Engine.h"

class ArrayView {
public:
	static bool show;
	static void* data;
	static std::string scrNm, varNm;
	static char type;
	static std::vector<int*> dims;
    static int windowSize;

    static void Draw();
};