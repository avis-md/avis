#pragma once
#include "importer_info.h"
#include <string>
#include <vector>

//LAMMPS dump file parser
class Lammps {
public:
	static bool Read(ParInfo* info);
	static bool ReadTrj(TrjInfo* info);

private:
	enum class ATTR {
		id,mol,type,element,
		x,y,z,xs,ys,zs,
		xu,yu,zu,xsu,ysu,zsu,
		CNT,
		SKIP
	};
	static const char* ATTRS[];

	static std::vector<std::string> string_split(std::string s, char c);
};