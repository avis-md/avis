// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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