#pragma once
#include "Engine.h"
#include "Particles.h"
#include "parloader.h"

//CDView molecular data file parser
class CDV {
public:
	static bool Read(ParInfo* info);
	static bool ReadTrj(TrjInfo* info);
};