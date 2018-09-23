#include "Particles.h"
#include "parloader.h"
#include "web/anweb.h"
#include "md/Protein.h"

Particles::paramdata::paramdata() {
	data = new float[particleSz]{};
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, particleSz * sizeof(float), 0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenTextures(1, &texBuf);
	glBindTexture(GL_TEXTURE_BUFFER, texBuf);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, buf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

Particles::paramdata::~paramdata() {
	delete[](data);
	glDeleteBuffers(1, &buf);
	glDeleteTextures(1, &texBuf);
}

void Particles::paramdata::Update() {
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleSz * sizeof(float), data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

std::vector<ResidueList> Particles::residueLists;
uint Particles::residueListSz;
uint Particles::particleSz;

std::string Particles::cfgFile, Particles::trjFile;

char* Particles::particles_Name, *Particles::particles_ResName;
glm::dvec3* Particles::particles_Pos, *Particles::particles_Vel;
short* Particles::particles_Typ;
byte* Particles::particles_Col;
Particles::conninfo Particles::particles_Conn;
float* Particles::particles_Rad;
Int2* Particles::particles_Res;

int Particles::particles_ParamSz = 0;
Particles::paramdata* Particles::particles_Params[] = {};
std::string Particles::particles_ParamNms[] = {};

std::vector<Particles::conninfo> Particles::particles_Conn2;

AnimData Particles::anim;

float Particles::boundingBox[] = {};

Vec3 Particles::colorPallete[] = {};
ushort Particles::defColPallete[] = {};
Vec4 Particles::_colorPallete[] = {};
byte Particles::defColPalleteSz = 0;
GLuint Particles::colorPalleteTex;
bool Particles::palleteDirty = false;

GLuint Particles::posVao;
GLuint Particles::posBuffer;
GLuint Particles::connBuffer;
GLuint Particles::colIdBuffer;
GLuint Particles::radBuffer;
GLuint Particles::posTexBuffer, Particles::connTexBuffer, Particles::colorIdTexBuffer, Particles::radTexBuffer;

void Particles::Init() {
	glGenVertexArrays(1, &posVao);
	glGenBuffers(1, &posBuffer);
	glGenBuffers(1, &connBuffer);
	glGenBuffers(1, &colIdBuffer);
	glGenBuffers(1, &radBuffer);

	glBindVertexArray(posVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &colorPalleteTex);
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_FLOAT, colorPallete);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Particles::UpdateColorTex() {
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGB, GL_FLOAT, colorPallete);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Particles::Clear() {
	if (particles_Pos) {
		residueLists.clear();
		
		delete[](particles_Name);
		delete[](particles_ResName);
		//delete[](particles_Pos);
		//delete[](particles_Vel);
		delete[](particles_Typ);
		delete[](particles_Col);
		std::free(particles_Conn.ids);
		particles_Pos = 0;
		/*
		for (auto& c : particles_Conn2) {
			delete[](c.ids);
		}
		*/
		residueListSz = particleSz = Particles::particles_Conn.cnt = 0;

		if (anim.poss) {
			delete[](anim.poss[0]);
			delete[](anim.vels[0]);
			delete[](anim.poss);
			delete[](anim.vels);
			if (anim.conns) {
				delete[](anim.conns[0]);
				delete[](anim.conns);
			}
		}
		/*
		for (auto& c : anim.conns2) {
			delete[](c.first);
			delete[](c.second[0]);
			delete[](c.second);
		}
		*/
		anim.frameCount = anim.activeFrame = 0;

		Protein::Clear();
	}
}

void Particles::GenTexBufs() {
	glGenTextures(1, &posTexBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, posTexBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, posBuffer);
	
	glGenTextures(1, &connTexBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, connTexBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, connBuffer);

	glGenTextures(1, &colorIdTexBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, colorIdTexBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R8, colIdBuffer);

	glGenTextures(1, &radTexBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, radTexBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, radBuffer);

	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void Particles::UpdateBufs() {
	for (auto& a : anim.conns2) {
		if (a.first) {
			delete[](a.first);
			delete[](a.second);
		}
		a.first = new uint[anim.frameCount]{};
		a.second = new Int2*[anim.frameCount]{};
	}

	std::vector<Vec3> poss(particleSz);
#pragma omp parallel for
	for (int a = 0; a < (int)particleSz; a++) {
		poss[a] = (Vec3)particles_Pos[a];
	}

	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, particleSz * sizeof(Vec3), &poss[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, connBuffer);
	glBufferData(GL_ARRAY_BUFFER, particles_Conn.cnt * 2 * sizeof(uint), particles_Conn.ids, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colIdBuffer);
	glBufferData(GL_ARRAY_BUFFER, particleSz * sizeof(byte), particles_Col, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, radBuffer);
	glBufferData(GL_ARRAY_BUFFER, particleSz * sizeof(float), particles_Rad, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particles::UpdateRadBuf() {
	glBindBuffer(GL_ARRAY_BUFFER, radBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleSz * sizeof(float), particles_Rad);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particles::UpdateConBufs2() {
	for (auto& c2 : particles_Conn2) {
		if (!c2.cnt) continue;
		if (!c2.buf) glGenBuffers(1, &c2.buf);
		glBindBuffer(GL_ARRAY_BUFFER, c2.buf);
		if (c2.ocnt < c2.cnt)
			glBufferData(GL_ARRAY_BUFFER, c2.cnt * sizeof(Int2), c2.ids, GL_DYNAMIC_DRAW);
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, c2.cnt * sizeof(Int2), c2.ids);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		c2.ocnt = c2.cnt;
		if (!c2.tbuf) {
			glGenTextures(1, &c2.tbuf);
		}
		glBindTexture(GL_TEXTURE_BUFFER, c2.tbuf);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, c2.buf);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
	}
}

void Particles::IncFrame(bool loop) {
	if (anim.activeFrame >= anim.frameCount - 1) {
		if (loop) SetFrame(0);
		else return;
	}
	else SetFrame(anim.activeFrame + 1);
}

void Particles::SetFrame(uint frm) {
	if (frm == anim.activeFrame) return;
	else {
		anim.activeFrame = frm;
		particles_Pos = anim.poss[anim.activeFrame];
		std::vector<Vec3> poss(particleSz);
#pragma omp parallel for
		for (int a = 0; a < (int)particleSz; a++) {
			poss[a] = (Vec3)particles_Pos[a];
		}
		glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, particleSz * sizeof(Vec3), &poss[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		bool has = false;
		for (int i = anim.conns2.size() - 1; i >= 0; i--) {
			auto& c2 = anim.conns2[i];
			if (!c2.first) continue;
			auto& c = particles_Conn2[i];
			c.cnt = c2.first[frm];
			c.ids = c2.second[frm];
			has = true;
		}
		if (has) UpdateConBufs2();
		AnWeb::OnAnimFrame();
		Scene::active->dirty = true;
	}
}

void Particles::AddParam() {
	particles_Params[particles_ParamSz] = new paramdata();
	particles_ParamNms[particles_ParamSz] = "Unnamed " + std::to_string(particles_ParamSz+1);
	particles_ParamSz++;
}

void Particles::RmParam(int i) {
	delete(particles_Params[i]);
	for (int a = i+1; a < particles_ParamSz; a++) {
		particles_Params[a-1] = particles_Params[a];
		particles_ParamNms[a-1] = particles_ParamNms[a];
	}
	particles_ParamSz--;
	particles_ParamNms[particles_ParamSz] = "";
}

void Particles::Serialize(XmlNode* nd) {
	nd->name = "Particles";
#define SVS(nm, vl) n->addchild(#nm, vl)
#define SV(nm, vl) SVS(nm, std::to_string(vl))
	auto n = nd->addchild("atoms");
	auto l = VisSystem::currentSavePath.find_last_of('/') + 1;
	auto s = VisSystem::currentSavePath.substr(0, l);
	auto s2 = cfgFile.substr(0, l);
	if (s == s2) {
		SVS(configuration, cfgFile.substr(l));
		n->children[0].params.emplace("relative", "1");
	}
	else {
		SVS(configuration, cfgFile);
		n->children[0].params.emplace("relative", "0");
	}
	s2 = trjFile.substr(0, l);
	if (s == s2) {
		SVS(trajectory, trjFile.substr(l));
		n->children[1].params.emplace("relative", "1");
	}
	else {
		SVS(trajectory, trjFile);
		n->children[1].params.emplace("relative", "0");
	}
	SerializeVis(n->addchild("visibility"));
	SerializeDM(n->addchild("drawmode"));

	n = nd->addchild("proteins");

}

void Particles::SerializeVis(XmlNode* nd) {
	nd->value = "par_vis.bin";
	std::ofstream strm(VisSystem::currentSavePath + "_data/par_vis.bin", std::ios::binary);
	_StreamWrite(&residueListSz, &strm, 4);
	for (auto& rls : residueLists) {
		char c = rls.visible ? 1 : 0;
		c += rls.expanded ? 2 : 0;
		strm.write(&c, 1);
		_StreamWrite(&rls.residueSz, &strm, 4);
		for (auto& rl : rls.residues) {
			c = rl.visible ? 1 : 0;
			c += rl.expanded ? 2 : 0;
			strm.write(&c, 1);
		}
	}
}

void Particles::SerializeDM(XmlNode* nd) {
	nd->value = "par_dm.bin";
	std::ofstream strm(VisSystem::currentSavePath + "_data/par_dm.bin", std::ios::binary);
	_StreamWrite(&residueListSz, &strm, 4);
	for (auto& rls : residueLists) {
		strm.write((char*)&rls.drawType, 1);
		_StreamWrite(&rls.residueSz, &strm, 4);
		for (auto& rl : rls.residues) {
			strm.write((char*)&rl.drawType, 1);
		}
	}
}

void Particles::Deserialize(XmlNode* nd) {
	auto l = VisSystem::currentSavePath.find_last_of('/') + 1;
	auto s = VisSystem::currentSavePath.substr(0, l);
	for (auto& n : nd->children) {
		if (n.name == "Particles") {
			for (auto& n2 : n.children) {
				if (n2.name == "atoms") {
					for (auto& n3 : n2.children) {
						if (n3.name == "configuration" && n3.value != "") {
							ParLoader::directLoad = true;
							if (n3.params["relative"] == "1")
								ParLoader::OnOpenFile(std::vector<std::string>{ s + n3.value });
							else
								ParLoader::OnOpenFile(std::vector<std::string>{ n3.value });
							//
							while (ParLoader::busy);;
						}
						else if (!!particleSz) {
							if (n3.name == "trajectory" && n3.value != "") {
								ParLoader::directLoad = true;
								if (n3.params["relative"] == "1")
									ParLoader::OnOpenFile(std::vector<std::string>{ s + n3.value });
								else
									ParLoader::OnOpenFile(std::vector<std::string>{ n3.value });

								//
								while (ParLoader::busy);;
							}
							else if (n3.name == "visibility") {
								DeserializeVis(&n3);
							}
							else if (n3.name == "drawmode") {
								DeserializeDM(&n3);
							}
						}
					}
				}
			}

			return;
		}
	}
}

void Particles::DeserializeVis(XmlNode* nd) {
	std::ifstream strm(VisSystem::currentSavePath2 + "_data/" + nd->value, std::ios::binary);
	uint32_t tmp;
	_Strm2Val(strm, tmp);
	if (tmp != residueListSz) {
		Debug::Warning("Particles::DeserializeVis", "residue list count is incorrect!");
		return;
	}
	for (auto& rls : residueLists) {
		char c;
		_Strm2Val(strm, c);
		rls.visible = !!(c & 1);
		rls.expanded = !!(c & 2);
		_Strm2Val(strm, tmp);
		if (tmp != rls.residueSz) {
			Debug::Warning("Particles::DeserializeVis", "residue count is incorrect!");
			return;
		}
		for (auto& rl : rls.residues) {
			_Strm2Val(strm, c);
			rl.visible = !!(c & 1);
			rl.expanded = !!(c & 2);
		}
	}
}

void Particles::DeserializeDM(XmlNode* nd) {
	std::ifstream strm(VisSystem::currentSavePath2 + "_data/" + nd->value, std::ios::binary);
	uint32_t tmp;
	_Strm2Val(strm, tmp);
	if (tmp != residueListSz) {
		Debug::Warning("Particles::DeserializeDM", "residue list count is incorrect!");
		return;
	}
	for (auto& rls : residueLists) {
		_Strm2Val(strm, rls.drawType);
		_Strm2Val(strm, tmp);
		if (tmp != rls.residueSz) {
			Debug::Warning("Particles::DeserializeDM", "residue count is incorrect!");
			return;
		}
		for (auto& rl : rls.residues) {
			_Strm2Val(strm, rl.drawType);
		}
	}
}