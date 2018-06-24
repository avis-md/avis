#pragma once
#include "Engine.h"
#include "parloader.h"

//GROMACS molecular data file parser
class Gromacs {
public:
	static bool Read(ParInfo* info);
	static bool ReadTrj(TrjInfo* info);

	static void _Read(const string& file, bool hasAnim);
	static bool _ReadTrj(const string& file); //short for trajectory(.trr)
};