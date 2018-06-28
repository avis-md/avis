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

void MDVBin::_Read(const string& file, bool hasAnim) {
	Particles::Clear();
	glGenVertexArrays(1, &Particles::posVao);
	glGenBuffers(1, &Particles::posBuffer);
	glGenBuffers(1, &Particles::connBuffer);
	glGenBuffers(1, &Particles::colIdBuffer);
	glGenBuffers(1, &Particles::radBuffer);

	std::ifstream strm(file, std::ios::binary);
	RD(Particles::particleSz);
	
	Particles::particles_Name = new char[Particles::particleSz * PAR_MAX_NAME_LEN]{};
	Particles::particles_ResName = new char[Particles::particleSz * PAR_MAX_NAME_LEN]{};
	Particles::residueLists = new ResidueList();
	Particles::residueListSz = 1;
	Particles::residueLists->residueSz = 1;
	Particles::residueLists->residues = new Residue();
	Particles::residueLists->residues->offset = 0;
	Particles::residueLists->residues->cnt = Particles::particleSz;
	Particles::residueLists->residues->offset_b = 0;
	Particles::residueLists->residues->cnt_b = 0;
	Particles::residueLists->residues->type = 255;

	if (!hasAnim) {
		Particles::particles_Pos = new Vec3[Particles::particleSz]{};
		Particles::particles_Vel = new Vec3[Particles::particleSz]{};
	}
	Particles::particles_Col = new byte[Particles::particleSz]{};
	Particles::particles_Rad = new float[Particles::particleSz]{};
	Particles::particles_Res = new Int2[Particles::particleSz]{};

	ushort rd;
	float px, py, pz, vx, vy, vz;
	for (uint i = 0; i < Particles::particleSz; i++) {
		RD(rd);
		RD(px);
		RD(py);
		RD(pz);
		RD(vx);
		RD(vy);
		RD(vz);
		if (!hasAnim) {
			Particles::particles_Pos[i] = Vec3(px, py, pz) * 0.1f;
			Particles::particles_Vel[i] = Vec3(vx, vy, vz) * 0.1f;
		}

		Particles::particles_Rad[i] = 4.0f - 2 * rd;
		Particles::particles_Res[i] = Int2(0, 0);
		Particles::particles_Col[i] = (byte)(rand() % 256);
	}

	glBindBuffer(GL_ARRAY_BUFFER, Particles::posBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::particleSz * sizeof(Vec3), Particles::particles_Pos, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::connBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::connSz * 2 * sizeof(uint), Particles::particles_Conn, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::colIdBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::particleSz * sizeof(byte), Particles::particles_Col, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::radBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::particleSz * sizeof(float), Particles::particles_Rad, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(Particles::posVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, Particles::posBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Particles::GenTexBufs();
}


bool MDVBin::_ReadTrj(const string& path) {
	auto& anm = Particles::anim;
	anm.reading = true;

	anm.frameCount = 100;
	anm.poss = new Vec3*[anm.frameCount];
	anm.vels = new Vec3*[anm.frameCount];

	anm.poss[0] = new Vec3[Particles::particleSz * anm.frameCount];
	memcpy(anm.poss[0], Particles::particles_Pos, sizeof(Vec3) * Particles::particleSz);

	anm.vels[0] = new Vec3[Particles::particleSz * anm.frameCount];
	memcpy(anm.vels[0], Particles::particles_Vel, sizeof(Vec3) * Particles::particleSz);

	uint ct;
	char dm[20];
	for (uint i = 1; i < anm.frameCount; i++) {
		std::stringstream sstrm;
		sstrm << std::setw(6) << std::setfill('0') << i + 2000;
		auto pp = path + sstrm.str() + ".bin";
		std::ifstream strm(pp, std::ios::binary);
		if (!strm.is_open()) {
			Debug::Warning("MDVBin", "cannot open bin file " + std::to_string(i) + "!");
			return false;
		}
		strm.read((char*)&ct, 4);
		if (ct != Particles::particleSz) {
			Debug::Warning("MDVBin", "particle count in bin file " + std::to_string(i) + " is incorrect!");
			return false;
		}
		anm.poss[i] = anm.poss[0] + i * Particles::particleSz;
		anm.vels[i] = anm.vels[0] + i * Particles::particleSz;
		for (uint j = 0; j < Particles::particleSz; j++) {
			auto& ps = anm.poss[i][j];
			auto& vl = anm.vels[i][j];
			strm.read(dm, 2);
			strm.read((char*)&ps, 12);
			strm.read((char*)&vl, 12);
			ps *= 0.1f;
		}
	}

	anm.reading = false;
	return !!Particles::anim.frameCount;
}