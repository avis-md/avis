#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "pdbx.h"

inline bool OFSTx(const char* c, const char* c2) {
	for (byte a = 0; a < 5; a++)
		if (c[a] != c2[a]) return false;
	return true;
}

#define ATM(c) OFSTx("ATOM ", c)

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

#define _F(nm, c)\
char* nm(char* f, char* l) {\
	while (f < l) {\
		if (*f c) return f;\
		f++;\
	}\
	return l;\
}

_F(NSPx, != ' ');
_F(SPx, == ' ');
_F(FDx, == '.');
_F(FQx, == '?');

bool PDBx::Read(ParInfo* info) {
	std::ifstream strm(info->path, std::ios::binary);
	if (!strm.is_open()) {
		SETERR("Cannot open file!");
		return false;
	}

	std::vector<std::string> lines;
	std::string cc;
	while (std::getline(strm, cc)) {
		if (ATM(cc.data())) {
			lines.push_back(cc);
		}
	}

	auto& sz = info->num = lines.size();
	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->resId = new uint16_t[sz];
	info->type = new uint16_t[sz];
	info->pos = new double[sz * 3];

	for (uint32_t i = 0; i < sz; ++i) {
		info->progress = i * 1.f / sz;
		char* ln = (char*)lines[i].data();
		char* n2 = SPx(ln + 6, ln + 20);
		n2 = NSPx(n2 + 1, n2 + 15);
		info->type[i] = (uint16_t)(*n2);
		char* n1 = NSPx(n2 + 2, n2 + 10);
		n2 = SPx(n1 + 1, n1 + 5);
		memcpy(&info->name[i * info->nameSz], n1, n2 - n1);
		n2 = FDx(n2 + 1, n2 + 10);
		memcpy(&info->resname[i * info->nameSz], n2 + 2, 3);
		info->resId[i] = (uint16_t)atoi(n2 + 12);
		n2 = FQx(n2 + 11, n2 + 25);
		info->pos[i * 3] = atof(n2 + 2) * 0.1f;
		n2 = SPx(n2 + 4, n2 + 30);
		n2 = NSPx(n2, n2 + 10);
		info->pos[i * 3 + 1] = atof(n2) * 0.1f;
		n2 = SPx(n2 + 2, n2 + 28);
		info->pos[i * 3 + 2] = atof(n2 + 1) * 0.1f;
	}
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
	return true;
}

bool PDBx::ReadFrm(FrmInfo* info) {
	return false;
}
