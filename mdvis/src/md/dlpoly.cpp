#include "dlpoly.h"
#include <fstream>
#include <stdint.h>

#define SETERR(msg) { memcpy(info->error, msg, sizeof(msg)); return false; }

bool DLPoly::Read(ParInfo* info) {
	char buf[500]{};
	std::ifstream strm(info->path);
	strm.getline(buf, 500);

	std::string dm;

	auto& sz = info->num;
	strm >> dm >> dm >> sz;

	strm >> dm;
	if (dm != "timestep") SETERR("timestep tag not found!")

	float vl;

	strm >> dm >> dm >> dm >> dm >> dm >> dm >> dm;
	strm >> vl >> dm >> dm;
	info->bounds[0] = -vl / 20;
	info->bounds[1] = vl / 20;
	strm >> dm >> vl >> dm;
	info->bounds[2] = -vl / 20;
	info->bounds[3] = vl / 20;
	strm >> dm >> dm >> vl;
	info->bounds[4] = -vl / 20;
	info->bounds[5] = vl / 20;

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz];
	info->pos = new float[sz * 3];
	info->vel = new float[sz * 3];

	int id;
	for (uint i = 0; i < sz; i++) {
		info->progress = i * 1.0f / sz;
		strm >> dm >> id;
		id--;
		memcpy(info->name, &dm[0], std::min<size_t>(info->nameSz, dm.size()));
		info->type[id] = dm[0];
		strm >> dm >> dm;
		strm >> vl;
		info->pos[id * 3] = vl / 10;
		strm >> vl;
		info->pos[id * 3 + 1] = vl / 10;
		strm >> vl;
		info->pos[id * 3 + 2] = vl / 10;
	}

	return true;
}