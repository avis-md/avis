#pragma once
#include "Engine.h"
#include "Particles.h"
#include "parloader.h"

//CDView molecular data file parser
class CDV {
public:
	static bool Read(ParInfo* info);
	static bool ReadTrj(TrjInfo* info);
	static void _Read(const string& file, bool hasAnim);
	static bool _ReadTrj(const string& file);
};