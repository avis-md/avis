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
#include "Engine.h"
#include "importer_info.h"

class GenericSSV {
	enum class TYPES : byte {
		NONE, ID, RID, RNM, TYP,
		POSX, POSY, POSZ,
		VELX, VELY, VELZ,
		ATTR
	};
	typedef std::vector<std::vector<double>> vecvecd;
public:
	static bool Read(ParInfo* info);
	static bool ReadFrm(FrmInfo* info);

	typedef std::pair<std::string, std::vector<double>> AttrTyp;
	static std::vector<AttrTyp> attrs;
	static vecvecd _attrs;
	static std::vector<vecvecd> _attrsf;
private:
	static std::vector<TYPES> _tps;
	static std::string _s;

	static void ParseTypes(const std::string& line, std::vector<TYPES>& ts);
};