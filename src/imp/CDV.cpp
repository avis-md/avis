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

#include "CDV.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

namespace {
	const char* bnms[] = {
		"box_sx", "box_ex",
		"box_sy", "box_ey",
		"box_sz", "box_ez"
	};

	std::vector<std::string> SplitString(std::string s, char c, bool rm) {
		std::vector<std::string> o = std::vector<std::string>();
		size_t pos = -1;
		do {
			s = s.substr(pos + 1);
			pos = s.find_first_of(c);
			if (!rm || pos > 0)
				o.push_back(s.substr(0, pos));
		} while (pos != std::string::npos);
		return o;
	}
}

bool CDV::Read(ParInfo* info) {
	char buf[500]{};
	std::ifstream strm(info->path);
	if (!strm) {
		SETERR("Cannot open file!");
		return false;
	}
	std::streampos sps;
	bool bfs[6] = {};
	int bn = 0;
	do {
		sps = strm.tellg();
		strm.getline(buf, 500);
		auto ss = SplitString(buf + 1, ' ', true);
		for (auto& s : ss) {
			auto s2 = SplitString(s, '=', false);
			if (s2.size() == 2) {
				for (int a = 0; a < 6; a++) {
					if (!bfs[a] && !std::strcmp(bnms[a], s2[0].data())) {
						info->bounds[a] = std::stod(s2[1]);
						bfs[a] = true;
						bn++;
						break;
					}
				}
			}
		}
	} while (buf[0] == '\'' || buf[0] == '#');

	strm.seekg(sps);

	uint32_t pi;
	auto& sz = info->num = 0;

	std::string s;

	auto pos = strm.tellg();
	std::getline(strm, s);
	while (s.back() == ' ') s.pop_back();
	auto ssz = SplitString(s, ' ', true).size();
	auto uid = (ssz == 6) || (ssz == 9);
	auto hvl = ssz > 7;
	strm.seekg(pos);

	uint32_t pi0 = 0xffff;

	while (std::getline(strm, s)) {
		if (s[0] == ' ') s = s.substr(s.find_first_not_of(' '));
		auto ps = s.find_first_of(' ');
		if (uid) ps = s.find_first_of(' ', ps + 1);
		pi = (uint32_t)std::stoi(s.substr(0, ps));
		sz = std::max(sz, pi + 1);
		pi0 = std::min(pi0, pi);
	}

	sz -= pi0;

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz]{};
	info->resId = new uint16_t[sz]{};
	info->pos = new double[sz * 3];
	info->vel = new double[sz * 3];

	size_t id;
	uint16_t rd;
	double vl;

	strm.clear();
	strm.seekg(sps);

	for (uint i = 0; i < sz; ++i) {
		info->progress = i * 1.f / sz;
		if (uid) strm >> rd;
		strm >> id >> rd;
		id -= pi0;
		info->resId[id] = rd;
		info->resname[id*info->nameSz] = '-';
		info->type[id] = *((uint16_t*)"H");
		info->name[id*info->nameSz] = 'A' + rd;
		strm >> vl;
		info->pos[id * 3] = vl;
		strm >> vl;
		info->pos[id * 3 + 1] = vl;
		strm >> vl;
		info->pos[id * 3 + 2] = vl;
		if (hvl) {
			strm >> vl;
			info->vel[id * 3] = vl;
			strm >> vl;
			info->vel[id * 3 + 1] = vl;
			strm >> vl;
			info->vel[id * 3 + 2] = vl;
		}
	}

	if (bn < 6) {
		auto& bnd = info->bounds;
		bnd[0] = bnd[1] = (float)info->pos[0];
		bnd[2] = bnd[3] = (float)info->pos[1];
		bnd[4] = bnd[5] = (float)info->pos[2];
		for (uint32_t i = 1; i < sz; ++i) {
			bnd[0] = std::min(bnd[0], info->pos[i * 3]);
			bnd[1] = std::max(bnd[1], info->pos[i * 3]);
			bnd[2] = std::min(bnd[2], info->pos[i * 3 + 1]);
			bnd[3] = std::max(bnd[3], info->pos[i * 3 + 1]);
			bnd[4] = std::min(bnd[4], info->pos[i * 3 + 2]);
			bnd[5] = std::max(bnd[5], info->pos[i * 3 + 2]);
		}
	}
	return true;
}

bool CDV::ReadFrame(FrmInfo* info) {
	std::string s;
	std::ifstream strm(info->path);
	if (!strm.is_open()) {
		SETERR("Cannot open file!");
		return false;
	}

	std::streampos sps;
	bool bfs[6] = {};
	int bn = 0;
	char buf[500]{};
	auto& bnd = *(info->bounds = new double[1][6]);
	do {
		sps = strm.tellg();
		strm.getline(buf, 500);
		auto ss = SplitString(buf + 1, ' ', true);
		for (auto& s : ss) {
			auto s2 = SplitString(s, '=', false);
			if (s2.size() == 2) {
				for (int a = 0; a < 6; a++) {
					if (!bfs[a] && !std::strcmp(bnms[a], s2[0].data())) {
						bnd[a] = std::stod(s2[1]);
						bfs[a] = true;
						bn++;
						break;
					}
				}
			}
		}
	} while (buf[0] == '\'' || buf[0] == '#');

	strm.seekg(sps);

	std::getline(strm, s);
	while (s.back() == ' ') s.pop_back();
	auto ssz = SplitString(s, ' ', true).size();
	auto uid = (ssz == 6) || (ssz == 9);
	auto hvl = ssz > 7;
	strm.seekg(sps);

	uint32_t pi0 = 0xffff;

	while (std::getline(strm, s)) {
		if (s[0] == ' ') s = s.substr(s.find_first_not_of(' '));
		auto ps = s.find_first_of(' ');
		if (uid) ps = s.find_first_of(' ', ps + 1);
		const auto pi = (uint32_t)std::stoi(s.substr(0, ps));
		pi0 = std::min(pi0, pi);
	}

	strm.clear();
	strm.seekg(sps);

	size_t id;
	std::string rd;
	double vl;
	for (uint32_t i = 0; i < info->parNum; ++i) {
		if (uid) strm >> rd;
		strm >> id >> rd;
		id -= pi0;
		if (id >= info->parNum) {
			SETERR("Index exceeds particle count!");
			return false;
		}
		strm >> vl;
		info->pos[id * 3] = vl;
		strm >> vl;
		info->pos[id * 3 + 1] = vl;
		strm >> vl;
		info->pos[id * 3 + 2] = vl;
		if (hvl) {
			strm >> vl;
			info->vel[id * 3] = vl;
			strm >> vl;
			info->vel[id * 3 + 1] = vl;
			strm >> vl;
			info->vel[id * 3 + 2] = vl;
		}
	}
	if (bn < 6) {
		bnd[0] = bnd[1] = (float)info->pos[0];
		bnd[2] = bnd[3] = (float)info->pos[1];
		bnd[4] = bnd[5] = (float)info->pos[2];
		for (uint32_t i = 1; i < info->parNum; ++i) {
			bnd[0] = std::min(bnd[0], info->pos[i * 3]);
			bnd[1] = std::max(bnd[1], info->pos[i * 3]);
			bnd[2] = std::min(bnd[2], info->pos[i * 3 + 1]);
			bnd[3] = std::max(bnd[3], info->pos[i * 3 + 1]);
			bnd[4] = std::min(bnd[4], info->pos[i * 3 + 2]);
			bnd[5] = std::max(bnd[5], info->pos[i * 3 + 2]);
		}
	}
	return true;
}