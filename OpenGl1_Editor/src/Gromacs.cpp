#pragma once
#include "Gromacs.h"

std::unordered_map<ushort, Vec3> Gromacs::_type2color = std::unordered_map<ushort, Vec3>();
std::unordered_map<uint, float> Gromacs::_bondLengths = std::unordered_map<uint, float>();

Gromacs::Gromacs(const string& file) : frames(), boundingBox() {
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
	frames.push_back(Frame());
	frames[0].count = 10000000;
	byte* buf = new byte[10000000 * sizeof(Vec3)]{};

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vboV);
	glGenBuffers(1, &_vboC);
	glBindBuffer(GL_ARRAY_BUFFER, _vboC);
	glBufferStorage(GL_ARRAY_BUFFER, 10000000 * sizeof(Vec3), buf, GL_DYNAMIC_STORAGE_BIT);

	for (uint i = 0; i < 10000000; i++)
		((Vec3*)buf)[i] = Vec3((i % 100) - 50, ((i % 10000) / 100) - 50, (i / 10000) - 50) * 0.1f;

	glBindBuffer(GL_ARRAY_BUFFER, _vboV);
	glBufferStorage(GL_ARRAY_BUFFER, 10000000 * sizeof(Vec3), buf, GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(_vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, _vboV);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, _vboC);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	delete[](buf);
}

void Gromacs::ReloadCols() {
	Vec4* cols = new Vec4[frames[0].count]{};
	for (uint i = 0; i < frames[0].count; i++) {
		*(Vec3*)&cols[i] = _type2color[frames[0].particles[i].atomId];
	}
	delete[](cols);
}

void Gromacs::LoadFiles() {
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
}