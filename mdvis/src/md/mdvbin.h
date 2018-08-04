#pragma once
#include "Engine.h"
#include "Particles.h"
#include "importer_info.h"

class MDVBin {
public:
	static bool Read(ParInfo* info);
	static bool ReadTrj(TrjInfo* info);
};