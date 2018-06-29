#include "Gromacs.h"
#include "vis/system.h"
#include "md/Protein.h"
#include "utils/rawvector.h"
#include "xdrfile/xdrfile.h"
#include "xdrfile/xdrfile_trr.h"

uint _find_char_not_of(char* first, char* last, char c) {
	for (char* f = first; first < last; first++) {
		if (*first != c)
			return first - f;
	}
	return -1;
}

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

bool Gromacs::Read(ParInfo* info) {
	std::ifstream strm(info->path, std::ios::binary);
	if (!strm.is_open()) {
		SETERR("Cannot open file!");
		return false;
	}

	char buf[100] = {};
	string s;

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
		info->progress = i * 1.0f / sz;
		strm.getline(buf, 100);
		if (strm.eof()) {
			SETERR("File data is incomplete!");
			return false;
		}
		uint n0 = _find_char_not_of(buf, buf + 5, ' ');
		info->resId[i] = (uint16_t)std::stoi(string(buf + n0, 5 - n0));
		memcpy(info->resname + i * info->nameSz, buf + 5, 5);
		n0 = _find_char_not_of(buf + 10, buf + 15, ' ');
		memcpy(info->name + i * info->nameSz, buf + 10 + n0, 5 - n0);
		info->type[i] = (uint16_t)buf[10 + n0];
		info->vel[i * 3] = std::stof(string(buf + 44, 8));
		info->vel[i * 3 + 1] = std::stof(string(buf + 52, 8));
		info->vel[i * 3 + 2] = std::stof(string(buf + 60, 8));
		info->pos[i * 3] = std::stof(string(buf + 20, 8));
		info->pos[i * 3 + 1] = std::stof(string(buf + 28, 8));
		info->pos[i * 3 + 2] = std::stof(string(buf + 36, 8));
	}

	strm >> info->bounds[1] >> info->bounds[3] >> info->bounds[5];
	return true;
}

bool Gromacs::ReadTrj(TrjInfo* info) {
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

void Gromacs::_Read(const string& file, bool hasAnim) {
	Particles::Clear();

	std::ifstream strm(file, std::ios::binary);
	if (!strm.is_open()) {
		Debug::Warning("Gromacs", "Cannot open file!");
		return;
	}

	Vec4* poss = 0, *cols = 0;
	char buf[100] = {};

	strm.getline(buf, 100);
	if (strm.eof()) {
		Debug::Warning("Gromacs", "Cannot read from file!");
		return;
	}
	string s(buf, 100);
	auto tp = string_find(s, "t=");
	if (tp > -1) {
		//frm.name = s.substr(0, tp);
		//frm.time = std::stof(s.substr(tp + 2));
	}
	else {
		//frm.name = s;
		//frm.time = 0;
	}
	strm.getline(buf, 100);
	Particles::particleSz = std::stoi(string(buf));
	Particles::particles_Name = new char[Particles::particleSz * PAR_MAX_NAME_LEN]{};
	Particles::particles_ResName = new char[Particles::particleSz * PAR_MAX_NAME_LEN]{};
	if (!hasAnim) {
		Particles::particles_Pos = new Vec3[Particles::particleSz];
		Particles::particles_Vel = new Vec3[Particles::particleSz];
	}
	Particles::particles_Col = new byte[Particles::particleSz];
	Particles::particles_Rad = new float[Particles::particleSz];
	Particles::particles_Res = new Int2[Particles::particleSz];

	auto rs = rawvector<ResidueList, uint>(Particles::residueLists, Particles::residueListSz);
	auto cn = rawvector<Int2, uint>(Particles::particles_Conn, Particles::connSz);

	uint64_t currResNm = -1, currResId = -1;
	ResidueList* trs = 0;
	Residue* tr = 0;
	auto rsv = rawvector<Residue, uint>(Particles::residueLists->residues, Particles::residueLists->residueSz);
	Vec3 vec;
	
	uint lastOff = 0, lastCnt = 0;

	for (uint i = 0; i < Particles::particleSz; i++) {
		strm.getline(buf, 100);
		//prt.residueNumber = std::stoul(string(buf, 5));
		uint64_t resId = *((uint64_t*)buf) & 0x000000ffffffffff;
		uint64_t resNm = *((uint64_t*)(buf + 5)) & 0x000000ffffffffff;
		memcpy(Particles::particles_ResName + i * PAR_MAX_NAME_LEN, buf + 5, 5);
		uint n0 = _find_char_not_of(buf + 10, buf + 15, ' ');
		memcpy(Particles::particles_Name + i * PAR_MAX_NAME_LEN, buf + 10 + n0, 5 - n0);
		//prt.atomId = (ushort)prt.atomName[0];
		//prt.atomNumber = std::stoul(string(buf + 15, 5));
		if (!hasAnim) {
			vec.x = std::stof(string(buf + 44, 8));
			vec.y = std::stof(string(buf + 52, 8));
			vec.z = std::stof(string(buf + 60, 8));
			Particles::particles_Vel[i] = vec;
			vec.x = std::stof(string(buf + 20, 8));
			vec.y = std::stof(string(buf + 28, 8));
			vec.z = std::stof(string(buf + 36, 8));
			Particles::particles_Pos[i] = vec;
		}
		if (currResNm != resNm) {
			rs.push(ResidueList());
			trs = &Particles::residueLists[Particles::residueListSz - 1];
			rsv = rawvector<Residue, uint>(trs->residues, trs->residueSz);
			trs->name = string(buf + 5, 5);
			currResNm = resNm;
		}

		auto id1 = Particles::particles_Name[i * PAR_MAX_NAME_LEN];
		if (currResId != resId) {
			if (!!i) {
				if (tr->type != 255) {
					lastOff = tr->offset;
					lastCnt = tr->cnt;
				}
				else {
					lastOff = i;
					lastCnt = 0;
				}
			}
			rsv.push(Residue());
			tr = &trs->residues[trs->residueSz-1];
			tr->offset = i;
			tr->offset_b = Particles::connSz;
			uint n02 = _find_char_not_of(buf, buf + 5, ' ');
			tr->name = string(buf + n02, 5 - n02);
			tr->type = AminoAcidType(buf + 5);
			tr->cnt = 0;
			tr->cnt_b = 0;
			currResId = resId;
		}
		if (!hasAnim) {
			for (uint j = 0; j < lastCnt + tr->cnt; j++) {
				Vec3 dp = Particles::particles_Pos[lastOff + j] - vec;
				auto dst = glm::length2(dp);
				if (dst < 0.0625) { //2.5A
					auto id2 = Particles::particles_Name[(lastOff + j) * PAR_MAX_NAME_LEN];
					float bst = VisSystem::_bondLengths[id1 + (id2 << 16)];
					if (dst < bst) {
						cn.push(Int2(i, lastOff + j));
						tr->cnt_b++;
					}
				}
			}
		}

		for (byte b = 0; b < Particles::defColPalleteSz; b++) {
			if (id1 == Particles::defColPallete[b]) {
				Particles::particles_Col[i] = b;
				break;
			}
		}

		float rad = VisSystem::radii[id1][1];
		
		Particles::particles_Rad[i] = rad;
		Particles::particles_Res[i] = Int2(Particles::residueListSz-1, trs->residueSz-1);

		tr->cnt++;
	}

	string bx;
	std::getline(strm, bx);
	auto spl = string_split(bx, ' ', true);
	Particles::boundingBox[1] = std::stof(spl[0]);
	Particles::boundingBox[3] = std::stof(spl[1]);
	Particles::boundingBox[5] = std::stof(spl[2]);

	Particles::UpdateBufs();

	//Particles::UpdateRadBuf();
	Particles::GenTexBufs();
	//Particles::UpdateColorTex();
}


bool Gromacs::_ReadTrj(const string& path) {

	int natoms = 0;
	
	auto file = xdrfile_open(path.c_str(), "rb");
	if (!file) return false;
	
	if (Particles::particles_Pos) {
		delete[](Particles::particles_Pos);
		Particles::particles_Pos = 0;
	}

	int step;
	float t, lambda;
	auto& anm = Particles::anim;
	anm.reading = true;
	Vec3* poss;

	auto frm = rawvector<Vec3*, uint>(anm.poss, anm.frameCount);

	for (bool ok;;) {
		poss = new Vec3[Particles::particleSz];
		ok = read_trr(file, &natoms, &step, &t, &lambda, 0, (float*)poss, 0, 0);
		if (!!ok) {
			delete[](poss);
			break;
		}
		if (!Particles::particles_Pos) {
			Particles::particles_Pos = poss;
		}
		frm.push(poss);
	}
	xdrfile_close(file);
	anm.reading = false;
	return !!Particles::anim.frameCount;
}