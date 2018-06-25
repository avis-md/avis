#include "importer_info.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "xdrfile/xdrfile.h"
#include "xdrfile/xdrfile_trr.h"

typedef unsigned int uint;

uint _find_char_not_of(char* first, char* last, char c) {
	for (char* f = first; first < last; first++) {
		if (*first != c)
			return first - f;
	}
	return -1;
}

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

EXPORT bool Read(ParInfo* info) {
	std::ifstream strm(info->path, std::ios::binary);
	if (!strm.is_open()) {
		SETERR("Cannot open file!");
		return false;
	}

	char buf[100] = {};
	std::string s;

	std::getline(strm, s);
	if (strm.eof()) {
		SETERR("Cannot read from file!");
		return false;
	}
	/*
	auto tp = string_find(s, "t=");
	if (tp > -1) {
		//frm.name = s.substr(0, tp);
		//frm.time = std::stof(s.substr(tp + 2));
	}
	else {
		//frm.name = s;
		//frm.time = 0;
	}
	*/
	std::getline(strm, s);
	auto& sz = info->num = std::stoi(s);
	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz];
	info->pos = new float[sz * 3];
	info->vel = new float[sz * 3];

	for (uint i = 0; i < sz; i++) {
		strm.getline(buf, 100);
		if (strm.eof()) {
			SETERR("File data is incomplete!");
			return false;
		}
		uint n0 = _find_char_not_of(buf, buf + 5, ' ');
		info->resId[i] = (uint16_t)std::stoi(std::string(buf + n0, 5 - n0));
		memcpy(info->resname + i * info->nameSz, buf + 5, 5);
		n0 = _find_char_not_of(buf + 10, buf + 15, ' ');
		memcpy(info->name + i * info->nameSz, buf + 10 + n0, 5 - n0);
		info->type[i] = (uint16_t)buf[10 + n0];
		info->vel[i * 3] = std::stof(std::string(buf + 44, 8));
		info->vel[i * 3 + 1] = std::stof(std::string(buf + 52, 8));
		info->vel[i * 3 + 2] = std::stof(std::string(buf + 60, 8));
		info->pos[i * 3] = std::stof(std::string(buf + 20, 8));
		info->pos[i * 3 + 1] = std::stof(std::string(buf + 28, 8));
		info->pos[i * 3 + 2] = std::stof(std::string(buf + 36, 8));
	}

	strm >> info->bounds[0] >> info->bounds[1] >> info->bounds[2];
	return true;
}

EXPORT bool ReadTrj(TrjInfo* info) {
	int natoms = 0;
	auto file = xdrfile_open(info->first, "rb");
	if (!file) {
		SETERR("Cannot open file!");
		return false;
	}

	int step;
	float t, lambda;
	float* _ps;
	std::vector<float*> poss;
	bool ok;
	do {
		_ps = new float[info->parNum * 3];
		ok = read_trr(file, &natoms, &step, &t, &lambda, 0, _ps, 0, 0);
		if (!!ok) {
			delete[](_ps);
			break;
		}
		poss.push_back(_ps);
		info->frames++;
	} while (info->frames != info->maxFrames);
	xdrfile_close(file);
	info->poss = new float*[info->frames];
	memcpy(info->poss, &poss[0], info->frames * sizeof(float));

	if (!info->frames) {
		SETERR("No frames contained in file!");
		return false;
	}
	else return true;
}