#include "CDV.h"
#include <sys/stat.h>
#include <iomanip>

inline bool file_exists(const std::string& name) {
#ifndef _WIN32
#define _stat stat
#endif
	struct _stat buf;
	return (_stat(name.c_str(), &buf) == 0);
}

bool CDV::Read(ParInfo* info) {
	char buf[500]{};
	std::ifstream strm(info->path);
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
		pi = (uint32_t)std::stoi(string_split(s, ' ')[0]);
		sz = std::max(sz, pi + 1);
	}

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz];
	info->pos = new float[sz * 3];
	info->vel = new float[sz * 3];

	uint16_t id, rd;
	float vl;

	strm.clear();
	strm.seekg(0, std::ios::beg);
	std::getline(strm, s);
	std::getline(strm, s);

	for (uint i = 0; i < sz; i++) {
		info->progress = i * 1.0f / sz;
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

	if (info->trajectory.maxFrames > 0) {
		info->trajectory.parNum = sz;
		ReadTrj(&info->trajectory);
	}
	return true;
}

#define ISNUM(c) (c >= '0' && c <= '9')

bool CDV::ReadTrj(TrjInfo* info) {
	std::string nmf = std::string(info->first);
	auto ps = nmf.find_last_of('/');
	auto nm = nmf.substr(ps + 1);
	int n1, n2 = 0;
	for (uint a = 0; a < nm.size() - 1; a++) {
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
	uint16_t frms = 0;
	std::vector<string> nms;
	do {
		std::stringstream sstrm;
		sstrm << std::setw(n1 - n2) << std::setfill('0') << st;
		std::string nm = nm1 + sstrm.str() + nm2;
		if (!file_exists(nm)) break;
		nms.push_back(nm);
		frms++;
		st += info->frameSkip;
	} while (frms != info->maxFrames);

	std::vector<float*> poss;
	poss.reserve(frms);
	string s;
	uint16_t id;
	string dm;
	float vl;
	for (auto& nm : nms) {
		info->progress = info->frames * 1.0f / frms;
		std::ifstream strm(nm);
		if (!strm.is_open()) break;
		poss.push_back(new float[info->parNum * 3]);
		auto ps = poss.back();

		std::getline(strm, s);
		std::getline(strm, s);
		for (uint j = 0; j < info->parNum; j++) {
			strm >> id >> dm;
			strm >> vl;
			ps[id * 3] = vl / 10;
			strm >> vl;
			ps[id * 3 + 1] = vl / 10;
			strm >> vl;
			ps[id * 3 + 2] = vl / 10;
		}
		info->frames++;
	}
	info->poss = new float*[info->frames];
	memcpy(info->poss, &poss[0], info->frames * sizeof(float*));

	if (!info->frames) {
		return false;
	}
	else return true;
}