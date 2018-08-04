#pragma once
#include "Engine.h"
#include "parloader.h"

//GROMACS molecular data file parser
class Gromacs {
public:
	static bool Read(ParInfo* info);
	static bool ReadTrj(TrjInfo* info);
};