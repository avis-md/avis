#include "CDV.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

bool CDV::Read(ParInfo* info) {
	char buf[500]{};
	std::ifstream strm(info->path);
	if (!strm) {
		SETERR("Cannot open file!");
		return false;
	}
	std::streampos sps;
	do {
		sps = strm.tellg();
		strm.getline(buf, 500);
	} while (buf[0] == '\'');

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

	while (std::getline(strm, s)) {
		auto ps = s.find_first_of(' ');
		if (uid) ps = s.find_first_of(' ', ps + 1);
		pi = (uint32_t)std::stoi(s.substr(0, ps));
		sz = std::max(sz, pi + 1);
	}

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz];
	info->pos = new double[sz * 3];
	info->vel = new double[sz * 3];

	size_t id;
	uint16_t rd;
	double vl;

	strm.clear();
	strm.seekg(0, std::ios::beg);
	std::getline(strm, s);
	std::getline(strm, s);

	for (uint i = 0; i < sz; ++i)  {
		info->progress = i * 1.f / sz;
		if (uid) strm >> rd;
		strm >> id >> rd;
		info->resId[id] = rd;
		info->resname[id*info->nameSz] = '-';
		info->type[id] = *((uint16_t*)"H");
		info->name[id*info->nameSz] = 'A' + rd;
		strm >> vl;
		info->pos[id * 3] = vl / 10;
		strm >> vl;
		info->pos[id * 3 + 1] = vl / 10;
		strm >> vl;
		info->pos[id * 3 + 2] = vl / 10;
		if (hvl) {
			strm >> vl;
			info->vel[id * 3] = vl / 10;
			strm >> vl;
			info->vel[id * 3 + 1] = vl / 10;
			strm >> vl;
			info->vel[id * 3 + 2] = vl / 10;
		}
	}

	auto& bnd = info->bounds;
	bnd[0] = bnd[1] = (float)info->pos[0];
	bnd[2] = bnd[3] = (float)info->pos[1];
	bnd[4] = bnd[5] = (float)info->pos[2];
	for (uint32_t i = 1; i < sz; ++i)  {
		bnd[0] = std::min(bnd[0], info->pos[i * 3]);
		bnd[1] = std::max(bnd[1], info->pos[i * 3]);
		bnd[2] = std::min(bnd[2], info->pos[i * 3 + 1]);
		bnd[3] = std::max(bnd[3], info->pos[i * 3 + 1]);
		bnd[4] = std::min(bnd[4], info->pos[i * 3 + 2]);
		bnd[5] = std::max(bnd[5], info->pos[i * 3 + 2]);
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
	std::getline(strm, s);
	std::getline(strm, s);

	auto pos = strm.tellg();
	std::getline(strm, s);
	while (s.back() == ' ') s.pop_back();
	auto ssz = SplitString(s, ' ', true).size();
	auto uid = (ssz == 6) || (ssz == 9);
	auto hvl = ssz > 7;
	strm.seekg(pos);

	size_t id;
	std::string rd;
	double vl;
	for (uint32_t i = 0; i < info->parNum; ++i)  {
		if (uid) strm >> rd;
		strm >> id >> rd;
		if (id >= info->parNum) {
			SETERR("Index exceeds particle count!");
			return false;
		}
		strm >> vl;
		info->pos[id * 3] = vl / 10;
		strm >> vl;
		info->pos[id * 3 + 1] = vl / 10;
		strm >> vl;
		info->pos[id * 3 + 2] = vl / 10;
		if (hvl) {
			strm >> vl;
			info->vel[id * 3] = vl / 10;
			strm >> vl;
			info->vel[id * 3 + 1] = vl / 10;
			strm >> vl;
			info->vel[id * 3 + 2] = vl / 10;
		}
	}
	return true;
}

std::vector<std::string> CDV::SplitString(std::string s, char c, bool rm) {
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