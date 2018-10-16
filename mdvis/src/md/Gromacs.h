#pragma once
#include <iostream>
#include <fstream>
#include "importer_info.h"

//GROMACS molecular data file parser
class Gromacs {
public:
	static bool Read(ParInfo* info);
	static bool ReadFrm(FrmInfo* info);
	static bool ReadTrj(TrjInfo* info);
private:
	static bool ReadGro2(ParInfo* info, std::ifstream& strm, size_t isz);
};