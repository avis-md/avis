#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "pdb.h"

bool ATM(char* c) {
	const char* c2 = "ATOM  ";
	for (int a = 0; a < 6; a++)
		if (c[a] != c2[a]) return false;
	return true;
}

char* NSP(char* f, char* l) {
	while (f < l) {
		if (*f != ' ') return f;
		f++;
	}
	return l;
}

char* SP(char* f, char* l) {
	while (f < l) {
		if (*f == ' ') return f;
		f++;
	}
	return l;
}

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

bool PDB::Read(ParInfo* info) {
	std::ifstream strm(info->path, std::ios::binary);
	if (!strm.is_open()) {
		SETERR("Cannot open file!");
		return false;
	}

	std::vector<char*> lines;
	char* cc = new char[150];
	while (strm.getline(cc, 150)) {
		if (ATM(cc)) {
			lines.push_back(cc);
			cc = new char[150];
		}
	}
	delete[](cc);

	auto& sz = info->num = lines.size();
	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->resId = new uint16_t[sz];
	info->type = new uint16_t[sz];
	info->pos = new float[sz * 3];

	for (unsigned int i = 0; i < sz; i++) {
		info->progress = i * 1.0f / sz;
		char* ln = lines[i];
		char* n1 = NSP(ln + 12, ln + 15);
		char* n2 = SP(n1, ln + 15);
		memcpy(&info->name[i * info->nameSz], n1, n2-n1);
		memcpy(&info->resname[i * info->nameSz], ln + 17, 3);
		info->resId[i] = (uint16_t)atoi(NSP(ln + 22, ln + 25));
		info->pos[i * 3] = (float)atof(NSP(ln + 30, ln + 37)) * 0.1f;
		info->pos[i * 3 + 1] = (float)atof(NSP(ln + 38, ln + 45)) * 0.1f;
		info->pos[i * 3 + 2] = (float)atof(NSP(ln + 46, ln + 53)) * 0.1f;
		if (*(ln + 76) == ' ') info->type[i] = (uint16_t)(*(ln + 77));
		else info->type[i] = *((uint16_t*)(ln + 76));
	}

	auto& bnd = info->bounds;
	bnd[0] = bnd[1] = info->pos[0];
	bnd[2] = bnd[3] = info->pos[1];
	bnd[4] = bnd[5] = info->pos[2];
	for (unsigned i = 1; i < sz; i++) {
		bnd[0] = std::min(bnd[0], info->pos[i * 3]);
		bnd[1] = std::max(bnd[1], info->pos[i * 3]);
		bnd[2] = std::min(bnd[2], info->pos[i * 3 + 1]);
		bnd[3] = std::max(bnd[3], info->pos[i * 3 + 1]);
		bnd[4] = std::min(bnd[4], info->pos[i * 3 + 2]);
		bnd[5] = std::max(bnd[5], info->pos[i * 3 + 2]);
	}
	return true;
}