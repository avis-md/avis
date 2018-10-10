#include "dlpoly.h"
#include <fstream>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <cstring>

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
	strm.ignore(500, '\n');

	double vl;
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
	info->pos = new double[sz * 3];
	info->vel = new double[sz * 3];

	std::vector<uint32_t> ignores;
	int li = 0;
	for (uint32_t id = 0; id != sz; id++) {
		info->progress = id * 1.0f / sz;
		strm >> dm;
		if (dm[0] == 'M' || dm[1] == 'M') {
			ignores.push_back(id);
			strm.ignore(500, '\n');
			strm.ignore(500, '\n');
			id--;
			sz--;
			li++;
			continue;
		}
		else li = 0;
		memcpy(info->name, &dm[0], std::min<size_t>(info->nameSz, dm.size()));
		info->type[id] = dm[0];
		strm >> dm >> dm >> dm;
		strm >> vl;
		info->pos[id * 3] = vl / 10;
		strm >> vl;
		info->pos[id * 3 + 1] = vl / 10;
		strm >> vl;
		info->pos[id * 3 + 2] = vl / 10;
	}

	ignores.push_back(-1);
	
	auto trj = &info->trajectory;
    std::vector<double*> poss = {};
    double* _ps;
    do {
		int ign = 0;
		dm = "";
		strm >> dm;
		if (dm != "timestep") goto out;

		strm.ignore(500, '\n');
		strm.ignore(500, '\n');
		strm.ignore(500, '\n');
		strm.ignore(500, '\n');
		
        _ps = new double[sz * 3];
        
        for (uint32_t a = 0; a != sz; a++) {
			trj->progress = a * 1.0f / sz;
			strm.ignore(500, '\n');
			if (ignores[ign] == a) {
				ign++;
				a--;
			}
			else {
				strm >> vl;
				_ps[a * 3] = vl / 10;
				strm >> vl;
				_ps[a * 3 + 1] = vl / 10;
				strm >> vl;
				_ps[a * 3 + 2] = vl / 10;
			}
			strm.ignore(500, '\n');
        }
		for (int a = 0; a < li; a++){
			strm.ignore(500, '\n');
			strm.ignore(500, '\n');
		}
        poss.push_back(_ps);
        trj->frames++;
    } while (trj->frames + 1 != trj->maxFrames);
out:
    if (!!trj->frames) {
        trj->frames++;
	    trj->poss = new double*[trj->frames];
        _ps = new double[sz*3];
        memcpy(_ps, info->pos, sz*3*sizeof(double));
        trj->poss[0] = _ps;
	    memcpy(trj->poss + 1, &poss[0], trj->frames * sizeof(double*));
    }

	return true;
}
