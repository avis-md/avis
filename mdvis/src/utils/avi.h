#pragma once
#include "Engine.h"

class AVI {
public:

	static void Encode(const string& path, byte** frames, byte frameC, int w, int h, byte chn);
};