#include "Gromacs.h"
#include "utils/rawvector.h"

std::unordered_map<uint, float> Gromacs::_bondLengths;
std::unordered_map<ushort, Vec3> Gromacs::_type2Col;

uint _find_char_not_of(char* first, char* last, char c) {
	for (char* f = first; first < last; first++) {
		if (*first != c)
			return first - f;
	}
	return -1;
}

void Gromacs::Read(const string& file) {
	Particles::Clear();
	glGenVertexArrays(1, &Particles::posVao);
	glGenBuffers(1, &Particles::posBuffer);
	glGenBuffers(1, &Particles::connBuffer);
	glGenBuffers(1, &Particles::colIdBuffer);

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
	Particles::particles_Pos = new Vec3[Particles::particleSz];
	Particles::particles_Vel = new Vec3[Particles::particleSz];
	Particles::particles_Col = new byte[Particles::particleSz];

	auto rs = rawvector<ResidueList, uint>(Particles::residueLists, Particles::residueListSz);
	auto cn = rawvector<Int2, uint>(Particles::particles_Conn, Particles::connSz);

	uint64_t currResNm = -1, currResId = -1;
	ResidueList* trs = 0;
	Residue* tr = 0;
	auto rsv = rawvector<Residue, uint>(Particles::residueLists->residues, Particles::residueLists->residueSz);
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
		Vec3 vec;
		vec.x = std::stof(string(buf + 44, 8));
		vec.y = std::stof(string(buf + 52, 8));
		vec.z = std::stof(string(buf + 60, 8));
		Particles::particles_Vel[i] = vec;
		vec.x = std::stof(string(buf + 20, 8));
		vec.y = std::stof(string(buf + 28, 8));
		vec.z = std::stof(string(buf + 36, 8));
		Particles::particles_Pos[i] = vec;

		if (currResNm != resNm) {
			rs.push(ResidueList());
			trs = &Particles::residueLists[Particles::residueListSz - 1];
			rsv = rawvector<Residue, uint>(trs->residues, trs->residueSz);
			trs->name = string(buf + 5, 5);
			currResNm = resNm;
		}

		if (currResId != resId) {
			rsv.push(Residue());
			tr = &trs->residues[trs->residueSz-1];
			tr->offset = i;
			tr->offset_b = Particles::connSz;
			uint n02 = _find_char_not_of(buf, buf + 5, ' ');
			tr->name = string(buf + n02, 5 - n02);
			tr->cnt = 0;
			tr->cnt_b = 0;
			currResId = resId;
		}
		else {
			for (uint j = 0; j < tr->cnt; j++) {
				Vec3 dp = Particles::particles_Pos[tr->offset + j] - vec;
				auto dst = glm::length2(dp);
				if (dst < 0.0625) { //2.5A
					auto id1 = Particles::particles_Name[i * PAR_MAX_NAME_LEN];
					auto id2 = Particles::particles_Name[j * PAR_MAX_NAME_LEN];
					float bst = _bondLengths[id1 + (id2 << 16)];
					if (dst < bst) {
						cn.push(Int2(i, tr->offset + j));
						tr->cnt_b++;
					}
				}
			}
		}

		tr->cnt++;
	}
	string bx;
	std::getline(strm, bx);
	auto spl = string_split(bx, ' ', true);
	Particles::boundingBox.x = std::stof(spl[0]);
	Particles::boundingBox.y = std::stof(spl[1]);
	Particles::boundingBox.z = std::stof(spl[2]);


	/*
	for (uint i = 0; i < Particles::particleSz - 1; i++) {
		auto pos = Particles::particles_Pos[i];
		auto id = (ushort)Particles::particles_Name[i * PAR_MAX_NAME_LEN];

		for (uint j = i + 1; j < Particles::particleSz; j++) {
			if (j != i) {
				auto ds = frm.particles[j].position - pos;
				auto dst = ds.x * ds.x + ds.y * ds.y + ds.z * ds.z;
				auto& n2 = frm.connSzs[j];
				if (dst < 0.0625) { //250pm
					auto id2 = frm.particles[j].atomId;
					float bst = _bondLengths[id + (id2 << 16)];
					if (dst < bst) {
						if (n < 4) frm.conns[i][n++] = j;
						if (n2 < 4) frm.conns[j][n2++] = i;
					}
				}
			}
		}
	}
	*/

	/*
	Particles::connSz = 5000000;
	Particles::particles_Conn = new Int2[5000000];


	for (uint i = 0; i < 10000000; i++) {
		Particles::particles_Pos[i] = Vec3((i % 100), ((i % 10000) / 100), (i / 10000)) * 0.1f;
		((int*)Particles::particles_Conn)[i] = i;
		Particles::particles_Col[i] = i % 3;
		memcpy(Particles::particles_Name + i * PAR_MAX_NAME_LEN, "OW", 3);// +std::to_string(i % 3);
		memcpy(Particles::particles_ResName + i * PAR_MAX_NAME_LEN, "WATER", 6);
	}
	*/

	glBindBuffer(GL_ARRAY_BUFFER, Particles::posBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::particleSz * sizeof(Vec3), Particles::particles_Pos, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::connBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::connSz * 2 * sizeof(uint), Particles::particles_Conn, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::colIdBuffer);
	glBufferData(GL_ARRAY_BUFFER, Particles::particleSz * sizeof(byte), Particles::particles_Col, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(Particles::posVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, Particles::posBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Particles::GenTexBufs();
}

void Gromacs::LoadFiles() {
	std::ifstream strm(IO::path + "/bondlengths.txt");
	_bondLengths.clear();
	if (strm.is_open()) {
		string s;
		while (!strm.eof()) {
			std::getline(strm, s);
			auto p = string_split(s, ' ', true);
			if (p.size() != 2) continue;
			auto p2 = string_split(p[0], '-');
			if (p2.size() != 2) continue;
			auto i1 = *(ushort*)&(p2[0])[0];
			auto i2 = *(ushort*)&(p2[1])[0];
			std::cout << std::hex << std::to_string(i1) << std::endl;
			auto ln = pow(std::stof(p[1]) * 0.001f, 2);
			_bondLengths.emplace(i1 + (i2 << 16), ln);
			_bondLengths.emplace(i2 + (i1 << 16), ln);
		}
		strm.close();
	}
	strm.open(IO::path + "/colors.txt");
	_type2Col.clear();
	if (strm.is_open()) {
		string s;
		Vec3 col;
		while (!strm.eof()) {
			std::getline(strm, s);
			auto p = string_split(s, ' ', true);
			if (p.size() != 4) continue;
			auto i = *(ushort*)&p[0];
			col.x = std::stof(p[1]);
			col.y = std::stof(p[2]);
			col.z = std::stof(p[3]);
			_type2Col.emplace(i, col);
		}
		strm.close();
	}
}