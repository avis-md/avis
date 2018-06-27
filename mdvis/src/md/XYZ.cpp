#include "XYZ.h"

bool XYZ::Read(ParInfo* info) {
	std::ifstream strm(info->path);
	if (!strm.is_open()) return false;
	strm >> info->num;
	auto& sz = info->num;

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz]{};
	info->pos = new float[sz * 3];

	string tp;
	strm >> tp;
	for (uint a = 0; a < sz; a++) {
		strm >> tp;
		info->type[a] = *((uint16_t*)(&tp[0]));
		strm >> info->pos[a * 3]
			>> info->pos[a * 3 + 1]
			>> info->pos[a * 3 + 2];
	}
	return true;
}

bool XYZ::ReadTrj(TrjInfo* info) {
	return false;
}