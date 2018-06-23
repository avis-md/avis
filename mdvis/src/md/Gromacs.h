#pragma once
#include "Engine.h"
#include "parloader.h"

//GROMACS molecular data file parser
class Gromacs {
public:
	static bool Read(ParInfo* info);
	static void Read(const string& file, bool hasAnim);
	static bool ReadTrj(const string& file); //short for trajectory(.trr)
};