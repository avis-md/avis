#include "Gromacs.h"
#include <string>
#include <vector>
#include "xdrfile/xdrfile.h"
#include "xdrfile/xdrfile_trr.h"
#include "xdrfile/xdrfile_xtc.h"

size_t _find_char_not_of(char* first, char* last, char c) {
	for (char* f = first; first < last; ++first) {
		if (*first != c)
			return first - f;
	}
	return -1;
}

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

bool Gromacs::Read(ParInfo* info) {
	std::ifstream strm(PATH(info->path), std::ios::binary);
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
	std::getline(strm, s);
	auto& sz = info->num = std::stoi(s);
	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz];
	info->pos = new double[sz * 3];
	info->vel = new double[sz * 3];

	auto lc = strm.tellg();
	strm.getline(buf, 100);
	auto ns = _find_char_not_of(buf, buf + 10, ' ') + 1;
	strm.seekg(lc);
	int of = 8;
	for (uint i = 0; i < sz; ++i) {
		info->progress = i * 1.f / sz;
		strm.getline(buf, 100);
		if (strm.eof()) {
			SETERR("File data is incomplete!");
			return false;
		}
		auto bf = buf;
		info->resId[i] = (uint16_t)std::atoi(bf); bf += ns;
		memcpy(info->resname + i * info->nameSz, bf, 5); bf += 5;
		auto n0 = _find_char_not_of(bf, bf + 5, ' ');
		memcpy(info->name + i * info->nameSz, bf + n0, 5 - n0);
		info->type[i] = (uint16_t)bf[n0];
		bf += 5 + ns;
		while (*(bf+of) != ' ') {
			of++;
		}
		info->pos[i * 3] = std::atof(bf); bf += of;
		info->pos[i * 3 + 1] = std::atof(bf); bf += of;
		info->pos[i * 3 + 2] = std::atof(bf); bf += of;
		if (!!bf) {
			info->vel[i * 3] = std::atof(bf); bf += of;
			info->vel[i * 3 + 1] = std::atof(bf); bf += of;
			info->vel[i * 3 + 2] = std::atof(bf);
		}
	}

	strm >> info->bounds[1] >> info->bounds[3] >> info->bounds[5];
	strm.ignore(10, '\n');
	if (!strm.eof())
		ReadGro2(info, strm, ns);
	return true;
}

bool Gromacs::ReadFrm(FrmInfo* info) {
	char buf[100]{};
	std::ifstream strm(info->path);
	if (!strm.is_open()) {
		SETERR("Cannot open file!");
		return false;
	}
	strm.getline(buf, 100);
	strm.getline(buf, 100);

	auto lc = strm.tellg();
	strm.getline(buf, 100);
	auto ns = _find_char_not_of(buf, buf + 10, ' ') + 1;
	strm.seekg(lc);
	int of = 8;
	for (uint i = 0; i < info->parNum; ++i) {
		strm.getline(buf, 100);
		if (strm.eof()) {
			SETERR("File data is incomplete!");
			return false;
		}
		auto bf = buf;
		bf += ns;
		bf += 5;
		bf += 5 + ns;
		while (*(bf+of) != ' ') {
			of++;
		}
		info->pos[i * 3] = std::atof(bf); bf += of;
		info->pos[i * 3 + 1] = std::atof(bf); bf += of;
		info->pos[i * 3 + 2] = std::atof(bf); bf += of;
		if (!!bf) {
			info->vel[i * 3] = std::atof(bf); bf += of;
			info->vel[i * 3 + 1] = std::atof(bf); bf += of;
			info->vel[i * 3 + 2] = std::atof(bf);
		}
	}
	return true;
}

bool Gromacs::ReadGro2(ParInfo* info, std::ifstream& strm, size_t isz) {
	auto trj = &info->trajectory;
	std::vector<double*> poss;
	double* ps;
	char buf[100] = {};
	isz = 10 + 2 * isz;
	while (strm.getline(buf, 100)) {
		info->trajectory.progress = 0.01f;
		ps = new double[info->num * 3];
		strm.ignore(100, '\n');
		for (uint32_t i = 0; i < info->num; ++i) {
			strm.getline(buf, 100);
			auto bf = buf + isz;
			ps[i * 3] = std::atof(bf); bf += 8;
			ps[i * 3 + 1] = std::atof(bf); bf += 8;
			ps[i * 3 + 2] = std::atof(bf);
		}
		poss.push_back(ps);
		trj->frames++;
		strm.ignore(100, '\n');
	}
	if (!trj->frames) return true;
	trj->poss = new double*[trj->frames];
	memcpy(trj->poss, &poss[0], trj->frames * sizeof(uintptr_t));
	return true;
}

bool Gromacs::ReadTrr(TrjInfo* info) {
	int natoms = 0;
	size_t fsz;
	{
		std::ifstream strm(info->first, std::ios::binary | std::ios::ate);
		fsz = strm.tellg();
	}
	read_trr_natoms(info->first, &natoms);
	if (natoms != info->parNum) {
		SETERR("Atom count is different!");
		return false;
	}

	auto file = xdrfile_open(info->first, "rb");
	if (!file) {
		SETERR("Cannot open file!");
		return false;
	}

	int step;
	float t, lambda;
	double* _ps;
	std::vector<double*> poss;
	bool ok;
	do {
		_ps = new double[info->parNum * 3];
		ok = read_trr(file, &natoms, &step, &t, &lambda, 0, _ps, 0, 0);
		if (!!ok) {
			delete[](_ps);
			break;
		}
		info->progress = ftell((FILE*)xdrfile_ptr(file)) / (float)fsz;
		poss.push_back(_ps);
		info->frames++;
	} while (info->frames != info->maxFrames);
	xdrfile_close(file);

	if (!info->frames) {
		SETERR("No frames contained in file!");
		return false;
	}
	info->poss = new double*[info->frames];
	memcpy(info->poss, &poss[0], info->frames * sizeof(uintptr_t));
	return true;
}

bool Gromacs::ReadXtc(TrjInfo* info) {
	int natoms = 0;
	size_t fsz;
	{
		std::ifstream strm(info->first, std::ios::binary | std::ios::ate);
		fsz = strm.tellg();
	}
	read_xtc_natoms((char*)info->first, &natoms);
	if (natoms != info->parNum) {
		SETERR("Atom count is different!");
		return false;
	}

	auto file = xdrfile_open(info->first, "rb");
	if (!file) {
		SETERR("Cannot open file!");
		return false;
	}

	int step;
	float t, lambda;
	double* _ps;
	std::vector<double*> poss;
	bool ok;
	do {
		_ps = new double[info->parNum * 3];
		std::vector<float> tmp(info->parNum * 3);
		matrix box;
		float prec;
		ok = read_xtc(file, natoms, &step, &t, box, (rvec*)tmp.data(), &prec);
		if (!!ok) {
			delete[](_ps);
			break;
		}
		for (int a = 0; a < info->parNum * 3; a++) {
			_ps[a] = tmp[a];
		}
		info->progress = ftell((FILE*)xdrfile_ptr(file)) / (float)fsz;
		poss.push_back(_ps);
		info->frames++;
	} while (info->frames != info->maxFrames);
	xdrfile_close(file);

	if (!info->frames) {
		SETERR("No frames contained in file!");
		return false;
	}
	info->poss = new double*[info->frames];
	memcpy(info->poss, &poss[0], info->frames * sizeof(uintptr_t));
	return true;
}