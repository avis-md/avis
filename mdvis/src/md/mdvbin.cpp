#include "mdvbin.h"
#include "vis/system.h"
#include "md/Protein.h"
#include <iomanip>
#include <sys/stat.h>

#define RD(val) strm.read((char*)(&val), sizeof(val))

bool MDVBin::Read(ParInfo* info) {
	std::ifstream strm(info->path, std::ios::binary);
	RD(info->num);
	auto& sz = info->num;

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz];
	info->pos = new float[sz * 3];
	info->vel = new float[sz * 3];

	for (uint i = 0; i < sz; i++) {
		info->progress = i * 1.0f / sz;
		RD(info->type[i]);
		RD(info->pos[i * 3]);
		RD(info->pos[i * 3 + 1]);
		RD(info->pos[i * 3 + 2]);
		RD(info->vel[i * 3]);
		RD(info->vel[i * 3 + 1]);
		RD(info->vel[i * 3 + 2]);
	}

	if (info->trajectory.maxFrames > 0) {
		info->trajectory.parNum = sz;
		ReadTrj(&info->trajectory);
	}

	return true;
}

#define ISNUM(c) (c >= '0' && c <= '9')

bool MDVBin::ReadTrj(TrjInfo* info) {
	std::string nmf = std::string(info->first);
	auto ps = nmf.find_last_of('/');
	auto nm = nmf.substr(ps + 1);
	int n1, n2 = 0;
	for (int a = 0; a < nm.size() - 1; a++) {
		if (nm[a + 1] == '.' && ISNUM(nm[a])) {
			n1 = a;
			break;
		}
	}
	for (int a = n1 - 1; a >= 0; a--) {
		if (!ISNUM(nm[a])) {
			n2 = a;
			break;
		}
	}
	int st = std::stoi(nm.substr(n2 + 1, n1 - n2));
	auto nm1 = nmf.substr(0, ps + 2 + n2);
	auto nm2 = nm.substr(n1 + 1);
	std::vector<float*> poss, vels;
	string s;
	uint32_t sz;
	uint16_t dm;

	std::vector<string> nms;
	do {
		struct stat _stat;
		std::stringstream sstrm;
		sstrm << std::setw(n1 - n2) << std::setfill('0') << st;
		string q = nm1 + sstrm.str() + nm2;
		if (stat(&q[0], &_stat) != 0) break;
		nms.push_back(q);

		info->frames++;
		st += info->frameSkip;
	} while (info->frames != info->maxFrames);

	uint a = 0;
	for (auto& q : nms) {
		info->progress = a * 1.0f / info->frames;
		std::ifstream strm(q, std::ios::binary);
		if (!strm.is_open()) {
			info->frames = a;
			break;
		}
		RD(sz);
		if (sz != info->parNum) {
			info->frames = a;
			break;
		}
		poss.push_back(new float[info->parNum * 3]);
		vels.push_back(new float[info->parNum * 3]);
		auto ps = poss.back();
		auto vs = vels.back();

		for (uint i = 0; i < info->parNum; i++) {
			RD(dm);
			RD(ps[i * 3]);
			RD(ps[i * 3 + 1]);
			RD(ps[i * 3 + 2]);
			RD(vs[i * 3]);
			RD(vs[i * 3 + 1]);
			RD(vs[i * 3 + 2]);
		}
		a++;
	}
	info->poss = new float*[info->frames];
	info->vels = new float*[info->frames];
	memcpy(info->poss, &poss[0], info->frames * sizeof(float));
	memcpy(info->vels, &vels[0], info->frames * sizeof(float));

	return !!info->frames;
}