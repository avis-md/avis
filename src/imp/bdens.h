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
#include <vector>
#include <string>

//minimal binary density data
/*
 byte  data     type       desc
 0~1   sizex    ushort     x dim length
 2~3   sizey    ushort     y dim length
 4~5   sizez    ushort     z dim length
 6     datatype char       D:d64, F:f32, S:i16, I:i32, L:i64
 7~?   density  datatype[] actual data
*/
class BDens {
public:
	static bool Read(ParInfo* info);
};
