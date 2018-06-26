#include "CDV.h"
#include "vis/system.h"
#include "md/Protein.h"
#include <iomanip>

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
	float dm;

	std::string s;
	while (std::getline(strm, s)) {
		pi = (uint32_t)std::stoi(string_split(s, ' ')[0]);
		Particles::particleSz = std::max(sz, pi + 1);
	}

	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz];
	info->resId = new uint16_t[sz];
	info->pos = new float[sz * 3];
	info->vel = new float[sz * 3];

	uint16_t id, rd;

	strm.clear();
	strm.seekg(0, std::ios::beg);
	std::getline(strm, s);
	std::getline(strm, s);

	for (uint i = 0; i < Particles::particleSz; i++) {
		strm >> id >> rd;
		info->resId[i] = id;
		info->type[id] = *((uint16_t*)"H");
		strm >> info->pos[id * 3]
			>> info->pos[id * 3 + 1]
			>> info->pos[id * 3 + 2];
	}
	return false;
}

#define ISNUM(c) (c >= '0' && c <= '9')

bool CDV::ReadTrj(TrjInfo* info) {
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
	int st = std::stoi(nm.substr(n2, n1 - n2 + 1));
	auto nm1 = nmf.substr(0, ps + 1 + n2);
	auto nm2 = nm.substr(n1 + 1);
	std::vector<float*> poss;
	string s;
	uint id, dm;
	do {
		std::stringstream sstrm;
		sstrm << std::setw(n1 - n2 + 1) << std::setfill('0') << st;
		std::ifstream strm(nm1 + sstrm.str() + nm2);
		if (!strm.is_open()) break;
		poss.push_back(new float[info->parNum * 3]);
		auto ps = poss.back();

		std::getline(strm, s);
		std::getline(strm, s);
		for (uint j = 0; j < Particles::particleSz; j++) {
			strm >> id >> dm;
			strm >> ps[id * 3]
				>> ps[id * 3 + 1]
				>> ps[id * 3 + 2];
		}

		st += info->frameSkip;
	} while (info->frames != info->maxFrames);
	info->poss = new float*[info->frames];
	memcpy(info->poss, &poss[0], info->frames * sizeof(float));

	if (!info->frames) {
		return false;
	}
	else return true;
}

void CDV::_Read(const string& file, bool hasAnim) {
	Particles::Clear();
	glGenVertexArrays(1, &Particles::posVao);
	glGenBuffers(1, &Particles::posBuffer);
	glGenBuffers(1, &Particles::connBuffer);
	glGenBuffers(1, &Particles::colIdBuffer);
	glGenBuffers(1, &Particles::radBuffer);

    char buf[500] {};
    std::ifstream strm(file);
    std::streampos sps;
    do {
        sps = strm.tellg();
        strm.getline(buf, 500);
    } while (buf[0] == '\'');

    strm.seekg(sps);

    uint pi;
	Particles::particleSz = 0;
    float dm;

	string s;
    while (std::getline(strm, s)) {
        pi = TryParse(string_split(s, ' ')[0], -1);
        Particles::particleSz = max(Particles::particleSz, pi+1);
    }
    std::cout << "ps " << Particles::particleSz << std::endl;

	Particles::particles_Name = new char[Particles::particleSz * PAR_MAX_NAME_LEN]{};
	Particles::particles_ResName = new char[Particles::particleSz * PAR_MAX_NAME_LEN]{};
    Particles::residueLists = new ResidueList();
    Particles::residueListSz = 1;
    Particles::residueLists->residueSz = 1;
    Particles::residueLists->residues = new Residue();
    Particles::residueLists->residues->offset = 0;
    Particles::residueLists->residues->cnt = Particles::particleSz;

	if (!hasAnim) {
		Particles::particles_Pos = new Vec3[Particles::particleSz]{};
		Particles::particles_Vel = new Vec3[Particles::particleSz]{};
	}
	Particles::particles_Col = new byte[Particles::particleSz]{};
	Particles::particles_Rad = new float[Particles::particleSz]{};
	Particles::particles_Res = new Int2[Particles::particleSz]{};
    
	uint id, rd;
	double px, py, pz;

	strm.clear();
	strm.seekg(0, std::ios::beg);
	std::getline(strm, s);
	std::getline(strm, s);

	for (uint i = 0; i < Particles::particleSz; i++) {
		strm >> id >> rd >> px >> py >> pz;

		if (!hasAnim) {
			Particles::particles_Pos[id] = Vec3(px, py, pz) * 0.1f;
		}

		Particles::particles_Rad[id] = 3.0f - rd;
		Particles::particles_Res[id] = Int2(0, 0);
		Particles::particles_Col[id] = (byte)(rand()%256);
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


bool CDV::_ReadTrj(const string& path) {
	auto& anm = Particles::anim;
	anm.reading = true;
    
    anm.frameCount = 140;
    anm.poss = new Vec3*[anm.frameCount];

	anm.poss[0] = new Vec3[Particles::particleSz * anm.frameCount];//Particles::particles_Pos;
	memcpy(anm.poss[0], Particles::particles_Pos, sizeof(Vec3) * Particles::particleSz);

	string s;
	uint id, dm;
	for (uint i = 1; i < anm.frameCount; i++) {
		std::stringstream sstrm;
		sstrm << std::setw(6) << std::setfill('0') << i * 100;
		std::ifstream strm(path + sstrm.str() + ".cdv");
		std::getline(strm, s);
		std::getline(strm, s);

		anm.poss[i] = anm.poss[0] + i * Particles::particleSz;
		for (uint j = 0; j < Particles::particleSz; j++) {
			auto& ps = anm.poss[i][j];
			strm >> id >> dm >> ps.x >> ps.y >> ps.z;
			ps *= 0.1f;
		}
	}

	anm.reading = false;
	return !!Particles::anim.frameCount;
}