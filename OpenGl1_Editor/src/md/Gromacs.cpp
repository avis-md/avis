#pragma once
#include "Gromacs.h"

void Gromacs::Read(const string& file) {
	/*
	std::ifstream strm(file, std::ios::binary);
	if (!strm.is_open()) {
		std::cout << "gromacs: cannot open file!" << std::endl;
		return;
	}

	Vec4* poss = 0, *cols = 0;
	char buf[100] = {};
	while (!strm.eof()) {
		strm.getline(buf, 100);
		if (strm.eof()) break;
		frames.push_back(Frame());
		auto& frm = frames.back();
		string s(buf, 100);
		auto tp = string_find(s, "t=");
		if (tp > -1) {
			frm.name = s.substr(0, tp);
			frm.time = std::stof(s.substr(tp + 2));
		}
		else {
			frm.name = s;
			frm.time = 0;
		}
		strm.getline(buf, 100);
		frm.count = std::stoi(string(buf));
		frm.particles.resize(frm.count);

		if (!poss) {
			poss = new Vec4[frm.count]{};
			cols = new Vec4[frm.count]{};
		}
		for (uint i = 0; i < frm.count; i++) {
			auto& prt = frm.particles[i];
			strm.getline(buf, 100);
			prt.residueNumber = std::stoul(string(buf, 5));
			prt.residueName = string(buf + 5, 5);
			prt.atomName = string(buf + 10, 5);
			while (prt.atomName[0] == ' ') prt.atomName = prt.atomName.substr(1);
			prt.atomId = (ushort)prt.atomName[0];
			prt.atomNumber = std::stoul(string(buf + 15, 5));
			prt.position.x = std::stof(string(buf + 20, 8));
			prt.position.y = std::stof(string(buf + 28, 8));
			prt.position.z = std::stof(string(buf + 36, 8));
			prt.velocity.x = std::stof(string(buf + 44, 8));
			prt.velocity.y = std::stof(string(buf + 52, 8));
			prt.velocity.z = std::stof(string(buf + 60, 8));
			if (!ubo_positions) {
				*(Vec3*)&poss[i] = prt.position;
				*(Vec3*)&cols[i] = _type2color[prt.atomId];
			}
		}
		string bx;
		std::getline(strm, bx);
		auto spl = string_split(bx, ' ', true);
		boundingBox.x = std::stof(spl[0]);
		boundingBox.y = std::stof(spl[1]);
		boundingBox.z = std::stof(spl[2]);

		frm.conns.resize(frm.count, glm::i32vec4(-1, -1, -1, -1));
		frm.connSzs.resize(frm.count);

		auto co = file + ".conn";
		bool cok = false;
		std::ifstream cstrm(co, std::ios::binary);
		if (cstrm.is_open()) {
			uint ccnt;
			cstrm >> ccnt;
			if (ccnt == frm.count) {
				std::cout << "using connection data file" << std::endl;
				string ss;
				for (uint i = 0; i < ccnt; i++) {
					for (uint j = 0; j < 4; j++)
						cstrm >> frm.conns[i][j];
				}
				cok = true;
			}
			cstrm.close();
		}
		if (!cok) {
			for (uint i = 0; i < frm.count - 1; i++) {
				auto pos = frm.particles[i].position;
				auto id = frm.particles[i].atomId;
				auto& n = frm.connSzs[i];
				for (uint j = i + 1; j < frm.count; j++) {
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
			std::ofstream costrm(co, std::ios::out | std::ios::binary);
			costrm << frm.count << "\n";
			for (uint i = 0; i < frm.count; i++) {
				for (uint j = 0; j < 4; j++)
					costrm << frm.conns[i][j] << " ";
				costrm << "\n";
			}
			costrm.flush();
			costrm.close();
		}
		if (!ubo_positions) {
#ifdef GRO_USE_COMPUTE
			ubo_positions = new ComputeBuffer<Vec4>(frm.count, poss);
			ubo_colors = new ComputeBuffer<Vec4>(frm.count, cols);
			ubo_conns = new ComputeBuffer<glm::i32vec4>(frm.count, &frm.conns[0]);
#else
			ubo_positions = new ShaderBuffer<Vec4>(frm.count, poss);
			ubo_colors = new ShaderBuffer<Vec4>(frm.count, cols);
#endif
			delete[](poss);
			delete[](cols);
		}
	}
	*/
	Particles::Clear();
	
	Particles::particleSz = 10000000;
	Particles::connSz = 5000000;
	Particles::particles_Name = new string[10000000];
	Particles::particles_ResName = new string[10000000];
	Particles::particles_Pos = new Vec3[10000000];
	//Particles::particles_Vel = new Vec3[10000000];
	Particles::particles_Col = new byte[10000000];

	glGenVertexArrays(1, &Particles::posVao);
	glGenBuffers(1, &Particles::posBuffer);
	glGenBuffers(1, &Particles::connBuffer);
	glGenBuffers(1, &Particles::colIdBuffer);

	uint* con = new uint[10000000];
	for (uint i = 0; i < 10000000; i++) {
		Particles::particles_Pos[i] = Vec3((i % 100), ((i % 10000) / 100), (i / 10000)) * 0.1f;
		con[i] = i;
		Particles::particles_Col[i] = i % 3;
		Particles::particles_Name[i] = "OW";// +std::to_string(i % 3);
		Particles::particles_ResName[i] = "WATER";
	}

	glBindBuffer(GL_ARRAY_BUFFER, Particles::posBuffer);
	glBufferData(GL_ARRAY_BUFFER, 10000000 * sizeof(Vec3), Particles::particles_Pos, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::connBuffer);
	glBufferData(GL_ARRAY_BUFFER, 10000000 * sizeof(uint), con, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, Particles::colIdBuffer);
	glBufferData(GL_ARRAY_BUFFER, 10000000 * sizeof(byte), Particles::particles_Col, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindVertexArray(Particles::posVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, Particles::posBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Particles::GenTexBufs();

	Particles::residueLists = new ResidueList[1000];
	Particles::residueListSz = 1000;

	for (uint i = 0; i < 1000; i++) {
		Particles::residueLists[i].name = "WATER";// +std::to_string(i);
		Particles::residueLists[i].residues = new Residue[100];
		Particles::residueLists[i].residueSz = 100;
		for (uint j = 0; j < 100; j++) {
			Particles::residueLists[i].residues[j].name = "W" + std::to_string(j);
			Particles::residueLists[i].residues[j].offset = i * 10000 + j * 100;
			Particles::residueLists[i].residues[j].cnt = 100;
		}
	}

	delete[](con);
}

void Gromacs::LoadFiles() {
	/*
	std::ifstream strm(IO::path + "/colors.txt", std::ios::binary);
	_type2color.clear();
	if (strm.is_open()) {
		string s;
		while (!strm.eof()) {
			std::getline(strm, s);
			auto p = string_split(s, ' ');
			if (p.size() != 4) continue;
			_type2color.emplace(*(ushort*)&p[0], Vec3(std::stof(p[1]), std::stof(p[2]), std::stof(p[3])));
		}
		strm.close();
	}
	strm.open(IO::path + "/bondlengths.txt", std::ios::binary);
	_bondLengths.clear();
	if (strm.is_open()) {
		string s;
		while (!strm.eof()) {
			std::getline(strm, s);
			auto p = string_split(s, ' ');
			if (p.size() != 2) continue;
			auto p2 = string_split(p[0], '-');
			if (p2.size() != 2) continue;
			auto i1 = *(ushort*)&p2[0];
			auto i2 = *(ushort*)&p2[1];
			auto ln = pow(std::stof(p[1]) * 0.001f, 2);
			_bondLengths.emplace(i1 + (i2 << 16), ln);
			_bondLengths.emplace(i2 + (i1 << 16), ln);
		}
		strm.close();
	}
	*/
}