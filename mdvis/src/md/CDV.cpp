#include "CDV.h"
#include "vis/system.h"
#include "md/Protein.h"
#include <iomanip>

void CDV::Read(const string& file, bool hasAnim) {
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
        Particles::particleSz = max(Particles::particleSz, pi);
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
		Particles::particles_Pos = new Vec3[Particles::particleSz];
		Particles::particles_Vel = new Vec3[Particles::particleSz];
	}
	Particles::particles_Col = new byte[Particles::particleSz];
	Particles::particles_Rad = new float[Particles::particleSz] {};
	Particles::particles_Res = new Int2[Particles::particleSz];
    
	uint id, rd;
	double px, py, pz;

	strm.clear();
    strm.seekg(sps);

	for (uint i = 0; i < Particles::particleSz; i++) {
		std::getline(strm, s);
		auto ss = string_split(s, ' ');
		id = TryParse(ss[0], -1);
		rd = TryParse(ss[1], 0.0f);
		px = TryParse(ss[2], 0.0f);
		py = TryParse(ss[3], 0.0f);
		pz = TryParse(ss[4], 0.0f);

		if (!hasAnim) {
			Particles::particles_Pos[id] = Vec3(px, py, pz) * 0.1f;
		}

		Particles::particles_Rad[id] = 3 - rd;
		Particles::particles_Res[id] = Int2(0, 0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, Particles::posBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::particleSz * sizeof(Vec3), Particles::particles_Pos, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::connBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::connSz * 2 * sizeof(uint), Particles::particles_Conn, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::colIdBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::particleSz * sizeof(byte), Particles::particles_Col, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, Particles::radBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::particleSz * sizeof(float), nullptr, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(Particles::posVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, Particles::posBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Particles::UpdateRadBuf();
	Particles::GenTexBufs();
}


bool CDV::ReadTrj(const string& path) {
	auto& anm = Particles::anim;
	anm.reading = true;
	Vec3* poss;
    
    anm.poss = new Vec3*[140];
    anm.frameCount = 140;

	anm.poss[0] = Particles::particles_Pos;

	string s;
	uint id;
	double px, py, pz;
	for (uint i = 1; i < 140; i++) {
		std::stringstream sstrm;
		sstrm << std::setw(6) << std::setfill('0') << i * 100;
		std::ifstream strm(path + sstrm.str() + ".cdv");
		std::getline(strm, s);
		std::getline(strm, s);

		anm.poss[i] = new Vec3[Particles::particleSz];
		for (uint j = 0; j < Particles::particleSz; j++) {
			std::getline(strm, s);
			auto ss = string_split(s, ' ');
			id = TryParse(ss[0], -1);
			px = TryParse(ss[2], 0.0f);
			py = TryParse(ss[3], 0.0f);
			pz = TryParse(ss[4], 0.0f);
			anm.poss[i][j] = Vec3(px, py, pz) * 0.1f;
		}
	}

	anm.reading = false;
	return !!Particles::anim.frameCount;
}