#include "lammps.h"
#include "importer_info.h"
#include <iostream>
#include <fstream>

const char* Lammps::ATTRS[] = {
    "id","mol","type","element",
	"x","y","z","xs","ys","zs",
	"xu","yu","zu","xsu","ysu","zsu"
};

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
	std::ifstream strm(info->path);
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
    info->pos = new float[sz * 3]{};
    info->vel = new float[sz * 3]{};
    
    strm.getline(buf, 100);
    SEQ(BOX BOUNDS);
    for (int a = 0; a < 6; a++)
        strm >> info->bounds[a];

    do {
        strm.getline(buf, 100);
    } while (buf[0] == 0);
    SEQ(ATOMS);
    auto tt = string_split(std::string(buf + sizeof("ITEM: ATOMS")), ' ');
    auto c = tt.size();
    std::vector<ATTR> attrs(c);
    for (size_t t = 0; t < c; t++) {
        auto ts = tt[t];
        for (int a = 0; a < (int)ATTR::CNT; a++) {
            if (ts == ATTRS[a]) {
                attrs[t] = (ATTR)a;
                goto fnd1;
            }
        }
        attrs[t] = ATTR::SKIP;
        fnd1:;
    }
    for (uint32_t a = 0; a < info->num; a++) {
        uint32_t x = a;
        for (auto t : attrs) {
            float tmp;
            std::string dummy;
            switch (t) {
                CS(id, strm >> x;)
                CS(type, strm >> info->type[x];)
                CS(x, strm >> info->pos[x*3];)
                CS(y, strm >> info->pos[x*3+1];)
                CS(z, strm >> info->pos[x*3+2];)
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
    std::vector<float*> poss = {};
    float* _ps;
    do {
        for (int a = 0; a < 10; a++) {
            do {
                strm.getline(buf, 100);
                if (strm.eof()) {
                    goto out;
                }
            } while (buf[0] == 0);
        }

        _ps = new float[info->num * 3];
        
        for (uint32_t a = 0; a < info->num; a++) {
            uint32_t x = a;
            for (auto t : attrs) {
                float tmp;
                std::string dummy;
                switch (t) {
                    CS(id, strm >> x;)
                    CS(x, strm >> _ps[x*3];)
                    CS(y, strm >> _ps[x*3+1];)
                    CS(z, strm >> _ps[x*3+2];)
                    CS(xs, strm >> tmp; _ps[x*3] = LP(info->bounds[0], info->bounds[1], tmp);)
                    CS(ys, strm >> tmp; _ps[x*3+1] = LP(info->bounds[2], info->bounds[3], tmp);)
                    CS(zs, strm >> tmp; _ps[x*3+2] = LP(info->bounds[4], info->bounds[5], tmp);)
                    default:
                        strm >> dummy;
                        break;
                }
            }
        }
        poss.push_back(_ps);
        trj->frames++;
    } while (trj->frames + 1 != trj->maxFrames);
out:
    if (!!trj->frames) {
        trj->frames++;
	    trj->poss = new float*[trj->frames];
        _ps = new float[info->num*3];
        memcpy(_ps, info->pos, info->num*3*sizeof(float));
        trj->poss[0] = _ps;
	    memcpy(trj->poss + 1, &poss[0], trj->frames * sizeof(uintptr_t));
    }

	return true;
}

std::vector<std::string> Lammps::string_split(std::string s, char c) {
	std::vector<std::string> o = {};
	size_t pos = -1;
	do {
		s = s.substr(pos + 1);
		pos = s.find_first_of(c);
		if (pos > 0)
			o.push_back(s.substr(0, pos));
	} while (pos != std::string::npos);
	return o;
}