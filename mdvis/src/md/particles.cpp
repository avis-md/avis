#include "particles.h"
#include "parloader.h"
#include "web/anweb.h"
#include "md/Protein.h"
#include "utils/glext.h"

uint Particles::AnimData::maxFramesInMem = 20;

void Particles::AnimData::AllocFrames(uint frames) {
	frameCount = frames;
	status.resize(frames, FRAME_STATUS::UNLOADED);
	poss.resize(frames);
	vels.resize(frames);
	paths.resize(frames);
}

void Particles::AnimData::Clear() {
	frameCount = currentFrame = 0;
	status.clear();
	poss.clear();
	vels.clear();
	conns.clear();
	conns2.clear();
	bboxs.clear();
	paths.clear();
}

void Particles::AnimData::Seek(uint f) {
	currentFrame = f;
	if (frameCount <= 1) return;
	if (status[f] != FRAME_STATUS::LOADED) {
		while (status[f] == FRAME_STATUS::READING){}
		if (status[f] == FRAME_STATUS::UNLOADED) {
			ParLoader::OpenFrameNow(f, paths[f]);
		}
		if (status[f] == FRAME_STATUS::BAD) return;
	}
	UpdateMemRange();
}

void Particles::AnimData::Update() {
	if (frameCount <= 1) return;
	if (maxFramesInMem > 1000000) return;

	if (maxFramesInMem < frameCount) {
		for (uint a = 0; a < frameMemPos; a++) {
			if (a + frameCount - frameMemPos < maxFramesInMem) continue;
			if (status[a] == FRAME_STATUS::LOADED) {
				std::vector<glm::dvec3>().swap(poss[a]);
				std::vector<glm::dvec3>().swap(vels[a]);
				status[a] = FRAME_STATUS::UNLOADED;
			}
		}
		for (uint a = frameMemPos + maxFramesInMem; a < frameCount; a++) {
			if (status[a] == FRAME_STATUS::LOADED) {
				std::vector<glm::dvec3>().swap(poss[a]);
				std::vector<glm::dvec3>().swap(vels[a]);
				status[a] = FRAME_STATUS::UNLOADED;
			}
		}
	}

	for (uint ff = currentFrame; ff < frameMemPos + maxFramesInMem; ff++) {
		auto f = Repeat(ff, 0U, frameCount);
		auto& st = status[f];
		if (st == FRAME_STATUS::READING || st == FRAME_STATUS::BAD) return;
		if (st == FRAME_STATUS::UNLOADED) {
			st = FRAME_STATUS::READING;
			ParLoader::OpenFrame(f, paths[f]);
			return;
		}
	}
	for (int f = currentFrame - 1; f >= (int)frameMemPos; f--) {
		auto& st = status[f];
		if (st == FRAME_STATUS::READING || st == FRAME_STATUS::BAD) return;
		if (st == FRAME_STATUS::UNLOADED) {
			st = FRAME_STATUS::READING;
			ParLoader::OpenFrame(f, paths[f]);
			return;
		}
	}
}

void Particles::AnimData::UpdateMemRange() {
	frameMemPos = (uint)std::max((int)currentFrame - (int)maxFramesInMem/2, 0);
}


Particles::paramdata::paramdata() {
	glGenBuffers(1, &buf);
	SetGLBuf<float>(buf, nullptr, particleSz);
	glGenTextures(1, &texBuf);
	glBindTexture(GL_TEXTURE_BUFFER, texBuf);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, buf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

Particles::paramdata::~paramdata() {
	glDeleteBuffers(1, &buf);
	glDeleteTextures(1, &texBuf);
}

void Particles::paramdata::Update() {
	if (!data.size()) return;
	float* d = timed? &data[particleSz*anim.currentFrame] : &data[0];
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleSz * sizeof(float), d);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	dirty = false;
}

uint Particles::residueListSz;
uint Particles::particleSz, Particles::_particleSz;

std::string Particles::cfgFile, Particles::trjFile;

glm::dvec3* Particles::poss, *Particles::vels;

std::vector<ResidueList> Particles::residueLists;

std::vector<char> Particles::names, Particles::resNames;
std::vector<short> Particles::types;
std::vector<byte> Particles::colors;
std::vector<float> Particles::radii;
std::vector<float> Particles::radiiscl;
std::vector<bool> Particles::visii;
std::vector<Int2> Particles::ress;
Particles::conninfo Particles::conns;

bool Particles::visDirty = false;

int Particles::particles_ParamSz = 0;
Particles::paramdata* Particles::particles_Params[] = {};
std::string Particles::particles_ParamNms[] = {};

std::vector<Particles::conninfo> Particles::particles_Conn2;

Particles::AnimData Particles::anim = {};

double Particles::boundingBox[] = {};

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

void Particles::Clear() {
	if (poss) {
		residueLists.clear();
		
		names.clear();
		resNames.clear();
		types.clear();
		colors.clear();
		radii.clear();
		radiiscl.clear();
		visii.clear();
		conns.ids.clear();
		poss = 0;
		residueListSz = particleSz = Particles::conns.cnt = 0;

		anim.Clear();

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

void Particles::Resize(uint i) {
	names.resize(i * PAR_MAX_NAME_LEN);
	resNames.resize(i * PAR_MAX_NAME_LEN);
	types.resize(i);
	colors.resize(i);
	radii.resize(i);
	radiiscl.resize(i, 1);
	visii.clear(); visii.resize(i, true);
	ress.resize(i);
}

void Particles::Update() {
	if (visDirty) {
		visDirty = false;
		Particles::UpdateRadBuf();
	}
	for (int a = 0; a < particles_ParamSz; a++) {
		auto& p = particles_Params[a];
		if (p->dirty) {
			p->Update();
		}
	}
	anim.Update();
}

void Particles::UpdateBufs() {
	for (auto& a : anim.conns2) {
		a.clear();
		a.resize(anim.frameCount);
	}

	std::vector<Vec3> ps(particleSz);
#pragma omp parallel for
	for (int a = 0; a < (int)particleSz; a++) {
		ps[a] = (Vec3)poss[a];
	}

	if (_particleSz != particleSz) {
		_particleSz = particleSz;
		SetGLBuf(posBuffer, &ps[0], particleSz, GL_DYNAMIC_DRAW);
		SetGLBuf(connBuffer, &conns.ids[0], particleSz);
		SetGLBuf(colIdBuffer, &colors[0], particleSz);
		SetGLBuf(radBuffer, &radii[0], particleSz);
	}
	else {
		SetGLSubBuf(posBuffer, &ps[0], particleSz);
		SetGLSubBuf(connBuffer, &conns.ids[0], particleSz);
		SetGLSubBuf(colIdBuffer, &colors[0], particleSz);
		UpdateRadBuf();
	}
}

void Particles::UpdateColorTex() {
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGB, GL_FLOAT, colorPallete);
	glBindTexture(GL_TEXTURE_2D, 0);
	Scene::dirty = true;
}

void Particles::UpdateRadBuf(int i) {
	if (i == -1) {
		std::vector<float> res(particleSz);
#pragma omp parallel for
		for (int a = 0; a < particleSz; a++) {
			res[a] = visii[a] ? radii[a]*radiiscl[a] : -1;
		}
		SetGLSubBuf(radBuffer, &res[0], particleSz);
	}
	else {
		glBindBuffer(GL_ARRAY_BUFFER, radBuffer);
		float vl = visii[i] ? radii[i]*radiiscl[i] : -1;
		glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(float), sizeof(float), &vl);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void Particles::UpdateConBufs2() {
	for (auto& c2 : particles_Conn2) {
		if (!c2.cnt) continue;
		if (!c2.buf) glGenBuffers(1, &c2.buf);
		if (c2.ocnt < c2.cnt)
			SetGLBuf(c2.buf, &c2.ids[0], c2.cnt, GL_DYNAMIC_DRAW);
		else
			SetGLSubBuf(c2.buf, &c2.ids[0], c2.cnt);
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
	if (anim.currentFrame >= anim.frameCount - 1) {
		if (loop) SetFrame(0);
		else return;
	}
	else SetFrame(anim.currentFrame + 1);
}

void Particles::SetFrame(uint frm) {
	if (frm == anim.currentFrame) return;
	else {
		if (anim.currentFrame != -1) {
			anim.Seek(frm);
			if (anim.status[frm] != AnimData::FRAME_STATUS::LOADED) return;
			poss = &anim.poss[anim.currentFrame][0];
			std::vector<Vec3> ps(particleSz);
	#pragma omp parallel for
			for (int a = 0; a < (int)particleSz; a++) {
				ps[a] = (Vec3)poss[a];
			}
			SetGLSubBuf(posBuffer, &ps[0], particleSz);
			for (int a = 0; a < particles_ParamSz; a++) {
				auto& p = particles_Params[a];
				if (p->timed) {
					p->Update();
				}
			}
			if (!!anim.bboxs.size()) memcpy(boundingBox, &anim.bboxs[6*frm], 6*sizeof(double));
		}
		else anim.Seek(frm);
		for (int i = anim.conns2.size() - 1; i >= 0; i--) {
			auto& c2 = anim.conns2[i];
			if (!c2.size()) continue;
			auto& c = particles_Conn2[i];
			c.cnt = c2[frm].count;
			c.ids = c2[frm].ids;
		}
		AnWeb::OnAnimFrame();
		if (!!anim.conns2.size()) UpdateConBufs2();
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
							ParLoader::busy = true;
							if (n3.params["relative"] == "1")
								ParLoader::OnOpenFile(std::vector<std::string>{ s + n3.value });
							else
								ParLoader::OnOpenFile(std::vector<std::string>{ n3.value });
							//
							while (ParLoader::busy){}
						}
						else if (!!particleSz) {
							if (n3.name == "trajectory" && n3.value != "") {
								ParLoader::directLoad = true;
								if (n3.params["relative"] == "1")
									ParLoader::OnOpenFile(std::vector<std::string>{ s + n3.value });
								else
									ParLoader::OnOpenFile(std::vector<std::string>{ n3.value });
								//
								//while (ParLoader::busy){}
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
	std::ifstream strm(VisSystem::currentSavePath2 + nd->value, std::ios::binary);
	if (!strm.is_open()) {
		Debug::Warning("Particles::DeserializeVis", "cannot open file!");
		return;
	}
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
	std::ifstream strm(VisSystem::currentSavePath2 + nd->value, std::ios::binary);
	if (!strm.is_open()) {
		Debug::Warning("Particles::DeserializeDM", "cannot open file!");
		return;
	}
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