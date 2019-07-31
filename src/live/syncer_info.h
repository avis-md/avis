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
#include <stdint.h>

#ifndef NO_EXPORT_IMP
#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif
typedef unsigned char byte;
typedef unsigned int uint;
#endif


struct SyncInfo {
	uint32_t num; //init only
	byte namesz; //IN
	char* resname; //init only
	char* name; //init only
	uint16_t* type; //init only
	uint16_t* resId; //init only
	float bounds[6]; //init only
	bool fill;
	float* pos;
	float* vel;
};