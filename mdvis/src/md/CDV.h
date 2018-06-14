#pragma once
#include "Engine.h"
#include "Particles.h"

//CDView molecular data file parser
class CDV {
public:
	static void Read(const string& file, bool hasAnim);
	static bool ReadTrj(const string& file);
};