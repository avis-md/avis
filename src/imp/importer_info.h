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
#include <cstring>

#ifndef NO_EXPORT_IMP
#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif
typedef unsigned char byte;
typedef unsigned int uint;
#endif

#ifdef _WIN32
#include <string>
#include <locale>
#include <codecvt>
inline std::wstring PATH(const std::string& s) {
	return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(s);
}
#else
#define PATH(s) s
#endif

struct TrjInfo {
	const char* first; //IN
	uint32_t parNum; //IN
	uint16_t maxFrames; //IN
	uint16_t frameSkip; //IN
	float progress;
	uint16_t frames;
	double** poss;
	double** vels;
	double (*bounds)[6];
	char error[100];
};

struct FrmInfo {
	const char* const path; //IN
	uint32_t const parNum; //IN
	double* const pos;
	double* const vel;
	double (*bounds)[6];
	char error[100];

	FrmInfo(const char* const path, 
			const uint32_t parNum, 
			double* const pos,
			double* const vel)
	: path(path), parNum(parNum), pos(pos), vel(vel), bounds(nullptr) {}
};

/*
info struct for the Load Configuration function.
Notes:
  trajectory is for data in the SAME file containing the configuration data.
  DO NOT open other files to load their trajectories. It will be done by the system.
  The waiting screen will not clear until this function returns.
  Either position data or density data must be provided, otherwise the import will have no effect.
*/
struct ParInfo {
	const char* path; //IN
	byte nameSz; //IN
	float progress; //OPT
	uint32_t num;
	char* resname; //OPT if densityNum > 0
	char* name; //OPT if densityNum > 0
	uint16_t* type; //OPT if densityNum > 0, H\0 if hydrogen
	uint16_t* resId; //OPT if densityNum > 0
	double* pos; //OPT if densityNum > 0
	double* vel; //OPT
	struct ProSec {
		enum TYPE { HELIX, SHEET } type;
		uint16_t resSt, resEd;
		byte helixPitch;
	}* secStructs; //OPT
	uint16_t secStructNum;
	double bounds[6];
	uint16_t densityNum[3];
	double* density; //OPT if num > 0
	TrjInfo trajectory; //OPT
	char error[100]; //OPT
};
