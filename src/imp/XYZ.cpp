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

#include "XYZ.h"

bool XYZ::Read(ParInfo* info) {
	std::ifstream strm(info->path);
	if (!strm.is_open()) return false;
	strm >> info->num;
	auto& sz = info->num;

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz]{};
	info->pos = new float[sz * 3];

	string tp;
	strm >> tp;
	for (uint a = 0; a < sz; ++a) {
		info->progress = a * 1.0f / sz;
		strm >> tp;
		info->type[a] = *((uint16_t*)(&tp[0]));
		strm >> info->pos[a * 3]
			>> info->pos[a * 3 + 1]
			>> info->pos[a * 3 + 2];
	}
	return true;
}

bool XYZ::ReadTrj(TrjInfo* info) {
	return false;
}