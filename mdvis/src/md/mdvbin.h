#pragma once
#include "Engine.h"
#include "Particles.h"

class MDVBin {
public:
	static void Read(const string& file, bool hasAnim);
	static bool ReadTrj(const string& file);
};