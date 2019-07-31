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

#include <iostream>
#include <fstream>
#include <vector>

class hdr {
public:
	static const char* szSignature, *szFormat;
	static unsigned char * read_hdr(const char* filename, unsigned int* w, unsigned int* h);
	static void write_hdr(const char* filename, unsigned int w, unsigned int h, unsigned char* data);
	static void to_float(unsigned char imagergbe[], int w, int h, float* res);
	static void to_rgbe(float* rgb, int w, int h, unsigned char* res);
};