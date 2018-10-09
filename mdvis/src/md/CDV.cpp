#include "CDV.h"
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
	while (std::getline(strm, s)) {
		pi = (uint32_t)std::stoi(s.substr(0, s.find(' ')));
		sz = std::max(sz, pi + 1);
	}

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz];
	info->pos = new double[sz * 3];
	info->vel = new double[sz * 3];

	uint16_t id, rd;
	double vl;

	strm.clear();
	strm.seekg(0, std::ios::beg);
	std::getline(strm, s);
	std::getline(strm, s);

	for (uint i = 0; i < sz; i++) {
		info->progress = i * 1.f / sz;
		strm >> id >> rd;
		info->resId[i] = id;
		info->type[id] = *((uint16_t*)"H");
			strm >> vl;
			info->pos[id * 3] = vl / 10;
			strm >> vl;
			info->pos[id * 3 + 1] = vl / 10;
			strm >> vl;
			info->pos[id * 3 + 2] = vl / 10;
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

	uint16_t id;
	std::string rd;
	double vl;
	for (uint32_t i = 0; i < info->parNum; i++) {
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
	}
	return true;
}