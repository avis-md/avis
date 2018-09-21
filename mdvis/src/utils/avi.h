#pragma once
#include "Engine.h"

class AVI {
public:

	static void Encode(const std::string& path, byte** frames, uint frameC, int w, int h, byte chn);
};