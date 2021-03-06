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

#include "lammps.h"
#include "importer_info.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>

const char* Lammps::ATTRS[] = {
	"id","mol","type","element",
	"x","y","z","xs","ys","zs",
	"xu","yu","zu","xsu","ysu","zsu"
};

#define SCL 0.1

#define LP(a,b,t) ((1-(t))*(a) + (t)*(b))

#define SETERR(msg) { memcpy(info->error, msg, sizeof(msg)); return false; }

bool EQ(char* c1, char* c2) {
	while (*c2 != 0) {
		if (*(c1++) != *(c2++)) return false;
	}
	return true;
}

#define SEQ(nm) if (!EQ(buf+6, (char*)#nm)) SETERR("Expected " #nm " tag!")

#define CS(tr,cmd) case ATTR::tr: \
	cmd \
	break;

bool Lammps::Read(ParInfo* info) {
	std::ifstream strm(info->path, std::ios::ate);
	auto maxspos = strm.tellg();
	strm.seekg(std::ios::beg);
	if (!strm.is_open())
		SETERR("Cannot open file!");

	char buf[100] = {};
	std::string s;

	strm.getline(buf, 100);
	if (strm.eof())
		SETERR("Cannot read from file!");
	if (!EQ(buf, (char*)"ITEM: "))
		SETERR("Item tag missing!");
	
	SEQ(TIMESTEP);
	strm.getline(buf, 100);

	strm.getline(buf, 100);
	SEQ(NUMBER OF ATOMS);
	strm.getline(buf, 100);
	auto& sz = info->num = atoi(buf);
	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz]{};
	info->resId = new uint16_t[sz]{};
	info->pos = new double[sz * 3]{};
	info->vel = new double[sz * 3]{};
	
	strm.getline(buf, 100);
	SEQ(BOX BOUNDS);
	for (int a = 0; a < 6; a++) {
		strm >> info->bounds[a];
		info->bounds[a] *= SCL;
	}

	strm.ignore(100, '\n');
	strm.getline(buf, 100);
	SEQ(ATOMS);
	auto tt = string_split(std::string(buf + sizeof("ITEM: ATOMS")), ' ');
	auto c = tt.size();
	std::vector<ATTR> attrs(c);
	for (size_t t = 0; t < c; ++t) {
		auto ts = tt[t];
		for (int a = 0; a < (int)ATTR::CNT; ++a) {
			if (ts == ATTRS[a]) {
				attrs[t] = (ATTR)a;
				goto fnd1;
			}
		}
		attrs[t] = ATTR::SKIP;
		fnd1:;
	}
	for (uint32_t a = 0; a < info->num; ++a) {
		uint32_t x = a;
		for (auto t : attrs) {
			float tmp;
			std::string dummy;
			switch (t) {
				CS(id, strm >> x; x--;)
				CS(type, strm >> info->type[x];)
				CS(x, strm >> info->pos[x*3];info->pos[x*3] *= SCL;)
				CS(y, strm >> info->pos[x*3+1];info->pos[x*3+1] *= SCL;)
				CS(z, strm >> info->pos[x*3+2];info->pos[x*3+2] *= SCL;)
				CS(xs, strm >> tmp; info->pos[x*3] = LP(info->bounds[0], info->bounds[1], tmp);)
				CS(ys, strm >> tmp; info->pos[x*3+1] = LP(info->bounds[2], info->bounds[3], tmp);)
				CS(zs, strm >> tmp; info->pos[x*3+2] = LP(info->bounds[4], info->bounds[5], tmp);)
				default:
					strm >> dummy;
					break;
			}
		}
	}
	
	auto trj = &info->trajectory;
	std::vector<double*> poss = {};
	std::vector<double> bounds = {};
	double* _ps;
	do {
		for (int a = 0; a < 6; ++a) {
			do {
				strm.getline(buf, 100);
				if (strm.eof()) {
					goto out;
				}
			} while (buf[0] == 0);
		}
		SEQ(BOX BOUNDS);
		bounds.resize((trj->frames+1)*6);
		auto bnds = bounds.data() + trj->frames * 6;
		for (int a = 0; a < 6; a++) {
			strm >> bnds[a];
			bnds[a] *= SCL;
		}

		strm.ignore(100, '\n');
		strm.getline(buf, 100);
		SEQ(ATOMS);
		
		trj->progress = ((float)strm.tellg()) / maxspos;
		_ps = new double[info->num * 3];
		
		for (uint32_t a = 0; a < info->num; ++a) {
			uint32_t x = a;
			for (auto t : attrs) {
				double tmp;
				std::string dummy;
				switch (t) {
					CS(id, strm >> x; x--;)
					CS(x, strm >> _ps[x*3]; _ps[x*3] *= SCL;)
					CS(y, strm >> _ps[x*3+1]; _ps[x*3+1] *= SCL;)
					CS(z, strm >> _ps[x*3+2]; _ps[x*3+2] *= SCL;)
					CS(xs, strm >> tmp; _ps[x*3] = LP(bnds[0], bnds[1], tmp);)
					CS(ys, strm >> tmp; _ps[x*3+1] = LP(bnds[2], bnds[3], tmp);)
					CS(zs, strm >> tmp; _ps[x*3+2] = LP(bnds[4], bnds[5], tmp);)
					default:
						strm >> dummy;
						break;
				}
			}
		}
		poss.push_back(_ps);
		trj->frames++;
	} while (trj->frames != trj->maxFrames);
out:
	if (!!trj->frames) {
		trj->poss = new double*[trj->frames];
		memcpy(trj->poss, &poss[0], trj->frames * sizeof(uintptr_t));
		trj->bounds = new double[trj->frames][6];
		for (uint16_t a = 0; a < trj->frames; ++a) {
			memcpy(trj->bounds[a], &bounds[a*6], 6*sizeof(double));
		}
	}

	return true;
}

std::vector<std::string> Lammps::string_split(std::string s, char c) {
	std::vector<std::string> o = {};
	size_t pos = -1;
	do {
		s = s.substr(pos + 1);
		pos = s.find_first_of(c);
		if (pos > 0) {
			const auto s2 = s.substr(0, pos);
			if (!std::all_of(s2.begin(), s2.end(), isspace))
				o.push_back(s2);
		}
	} while (pos != std::string::npos);
	return o;
}
