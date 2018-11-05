#include "particles.h"
#include "parloader.h"
#include "GenericSSV.h"
#include "Protein.h"
#include "web/anweb.h"
#include "utils/glext.h"

Particles::paramdata::paramdata() {
	glGenBuffers(1, &buf);
	SetGLBuf<float>(buf, nullptr, particleSz);
	glGenTextures(1, &texBuf);
	glBindTexture(GL_TEXTURE_BUFFER, texBuf);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, buf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	ApplyParCnt();
	ApplyFrmCnt();
}

Particles::paramdata::~paramdata() {
	glDeleteBuffers(1, &buf);
	glDeleteTextures(1, &texBuf);
}

std::vector<double>& Particles::paramdata::Get(uint frm) {
	if (!frm || !timed) return data;
	else return dataAll[frm-1];
}

void Particles::paramdata::ApplyParCnt() {
	if (!particleSz) return;
	SetGLBuf<float>(buf, nullptr, particleSz);
}

void Particles::paramdata::ApplyFrmCnt() {
	if (Particles::anim.frameCount < 2) return;
	dataAll.resize(Particles::anim.frameCount-1);
}

void Particles::paramdata::Update() {
	auto& dt = Get(anim.currentFrame);
	auto sz = (int)dt.size();
	if (!sz) return;
	SetGLSubBuf<>(buf, dt.data(), particleSz);
	dirty = false;
}

void Particles::paramdata::Clear() {
	timed = false;
	std::vector<double>().swap(data);
	std::vector<std::vector<double>>().swap(dataAll);
}

std::string Particles::paramdata::Export() {
	std::ostringstream strm;
	if ((!timed && !data.size())) {
		return "0";
	}
	strm << Particles::particleSz << " " << (timed? Particles::anim.frameCount : 1) << "\n";
	if (timed) {
		for (auto& d : dataAll) {
			strm << d.size() << " ";
			for (auto& d2 : d) {
				strm << d2 << " ";
			}
		}
		for (int a = 0; a < Particles::anim.frameCount; a++) {
			auto& d = Get(a);
			strm << d.size() << " ";
			for (auto& d2 : d) {
				strm << d2 << " ";
			}
		}
	}
	else {
		for (auto& d : data) {
			strm << d << " ";
		}
	}
	return strm.str();
}

void Particles::paramdata::Import(const std::string& buf) {
	std::istringstream strm(buf);
	int psz;
	strm >> psz;
	if (!psz) return;
	else if (psz != Particles::particleSz) {
		Debug::Warning("Attr::Import", "not atom count!");
		return;
	}
	strm >> psz;
	if (psz != 1 && psz != Particles::anim.frameCount) {
		Debug::Warning("Attr::Import", "not frame count!");
		return;
	}

	timed = (psz > 1);
	if (timed) {
		int n;
		for (int f = 0; f < psz; ++f) {
			strm >> n;
			if (!n) continue;
			auto& d = Get(f);
			d.resize(Particles::particleSz);
			for (auto& d2 : d) {
				strm >> d2;
			}
		}
	}
	else {
		data.resize(Particles::particleSz);
		for (auto& d : data) {
			strm >> d;
		}
	}
	dirty = true;
}


uint Particles::AnimData::maxFramesInMem = 20;

void Particles::AnimData::AllocFrames(uint frames) {
	frameCount = frames;
	status.resize(frames, FRAME_STATUS::UNLOADED);
	poss.resize(frames);
	vels.resize(frames);
	paths.resize(frames);
}

void Particles::AnimData::FillBBox() {
	bboxs.resize(frameCount * 6);
	for (int a = 0; a < frameCount; a++) {
		memcpy(&bboxs[a*6], boundingBox, 6*sizeof(double));
	}
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
	if (std::this_thread::get_id() == Engine::_mainThreadId) {
		for (auto& a : attrs) {
			a->Update();
		}
	}
	UpdateMemRange();
}

void Particles::AnimData::Update() {
	if (frameCount <= 1) return;

	if (dirty) {
		dirty = false;
		for (auto& a : attrs) {
			a->ApplyFrmCnt();
		}
	}

	if (maxFramesInMem < 1000000) {
		if (maxFramesInMem < frameCount) {
			for (uint a = 0; a < frameMemPos; ++a) {
				if (a + frameCount - frameMemPos < maxFramesInMem) continue;
				if (status[a] == FRAME_STATUS::LOADED) {
					std::vector<glm::dvec3>().swap(poss[a]);
					std::vector<glm::dvec3>().swap(vels[a]);
					status[a] = FRAME_STATUS::UNLOADED;
				}
			}
			for (uint a = frameMemPos + maxFramesInMem; a < frameCount; ++a) {
				if (status[a] == FRAME_STATUS::LOADED) {
					std::vector<glm::dvec3>().swap(poss[a]);
					std::vector<glm::dvec3>().swap(vels[a]);
					status[a] = FRAME_STATUS::UNLOADED;
				}
			}
		}

		for (uint ff = currentFrame; ff < frameMemPos + maxFramesInMem; ++ff) {
			auto f = Repeat(ff, 0U, frameCount);
			auto& st = status[f];
			if (st == FRAME_STATUS::READING || st == FRAME_STATUS::BAD) return;
			if (st == FRAME_STATUS::UNLOADED) {
				st = FRAME_STATUS::READING;
				ParLoader::OpenFrame(f, paths[f]);
				return;
			}
		}
		for (int f = currentFrame - 1; f >= (int)frameMemPos; --f) {
			auto& st = status[f];
			if (st == FRAME_STATUS::READING || st == FRAME_STATUS::BAD) return;
			if (st == FRAME_STATUS::UNLOADED) {
				st = FRAME_STATUS::READING;
				ParLoader::OpenFrame(f, paths[f]);
				return;
			}
		}
	}
}

void Particles::AnimData::UpdateMemRange() {
	frameMemPos = (uint)std::max((int)currentFrame - (int)maxFramesInMem/2, 0);
}


byte Particles::SpecificColor::nextId = 255;

Particles::SpecificColor::SpecificColor() {
	colId = nextId--;
	_colorPallete[colId] = white();
	di = Popups::DropdownItem(&resFlags, reslist.data());
	di.flags = true;
}

void Particles::SpecificColor::Update() {
	if (_type != type || _resFlags != resFlags) {
		Revert();
		_type = type;
		_resFlags = resFlags;
		if (!type.size() || !resFlags) return;
		UpdateMask();
		UpdateColorTex();
	}
	else if (_col != _colorPallete[colId]) {
		_col = _colorPallete[colId];
		if (!type.size() || !resFlags) return;
		UpdateColorTex();
	}
}

void Particles::SpecificColor::UpdateMask() {
	type.resize(std::min<size_t>(type.size(), PAR_MAX_NAME_LEN));
	auto rls = reslist.size();
	for (auto& rli : residueLists) {
		for (int a = 0; a < rls; a++) {
			if (!!(resFlags & (1 << a))) {
				if (rli.name == reslist[a]) goto use;
			}
		}
		continue;
	use:
		uint first = rli.residues[0].offset;
		uint last = rli.residues.back().offset + rli.residues.back().cnt;
		for (uint a = first; a < last; a++) {
			auto nm = &names[a*PAR_MAX_NAME_LEN];
			if (!std::string(nm, PAR_MAX_NAME_LEN).compare(0, type.size(), type)) {
				mask.push_back(a);
				colors[a] = colId;
			}
		}
	}
	SetGLSubBuf(colIdBuffer, &colors[0], particleSz);
}

void Particles::SpecificColor::Revert() {

	mask.clear();
}


uint Particles::residueListSz;
uint Particles::particleSz, Particles::_particleSz;

std::string Particles::cfgFile, Particles::trjFile;

glm::dvec3* Particles::poss, *Particles::vels;

std::vector<ResidueList> Particles::residueLists;
std::vector<std::string> Particles::reslist;

std::vector<char> Particles::names, Particles::resNames;
std::vector<short> Particles::types;
std::vector<byte> Particles::colors;
std::vector<float> Particles::radii;
std::vector<float> Particles::radiiscl;
std::vector<bool> Particles::visii;
std::vector<Int2> Particles::ress;
Particles::conninfo Particles::conns;

bool Particles::bufDirty = false, Particles::visDirty = false, Particles::palleteDirty = false;

std::vector<Particles::paramdata*> Particles::attrs;
std::vector<std::string> Particles::attrNms;

std::vector<Particles::conninfo> Particles::particles_Conn2;

Particles::AnimData Particles::anim = {};

double Particles::boundingBox[] = {};
glm::dvec3 Particles::bboxCenter;
bool Particles::boxPeriodic = false;

std::vector<Particles::DefColor> Particles::defColors;
std::vector<std::pair<ushort, Vec3>> Particles::colorPallete;
Vec4 Particles::_colorPallete[256];
std::vector<Particles::SpecificColor> Particles::colorOverrides;
GLuint Particles::colorPalleteTex;

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_FLOAT, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	attrNms.reserve(50); //reallocation will break something, temporary for now
	attrNms.push_back("");
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
		poss = vels = nullptr;
		for (auto& a : attrs) {
			delete(a);
		}
		attrs.clear();
		attrNms.clear();
		attrNms.push_back("");
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
	particleSz = i;
	names.resize(i * PAR_MAX_NAME_LEN);
	resNames.resize(i * PAR_MAX_NAME_LEN);
	types.resize(i);
	colors.resize(i);
	radii.resize(i, 1);
	radiiscl.resize(i, 1);
	visii.resize(i, true);
	ress.resize(i);

	for (auto& a : attrs) {
		a->ApplyParCnt();
	}
}

void Particles::Update() {
	if (bufDirty) {
		bufDirty = false;
		UpdateBufs();
	}
	if (visDirty) {
		visDirty = false;
		UpdateRadBuf();
	}
	anim.Update();
	for (auto& a : attrs) {
		if (a->dirty) {
			a->Update();
		}
	}
}

void Particles::UpdateBBox() {
	bboxCenter = glm::dvec3((boundingBox[1] + boundingBox[0]) / 2,
		(boundingBox[3] + boundingBox[2]) / 2,
		(boundingBox[5] + boundingBox[4]) / 2);
}

void Particles::UpdateBufs() {
	for (auto& a : anim.conns2) {
		a.clear();
		a.resize(anim.frameCount);
	}

	std::vector<Vec3> ps(particleSz);
#pragma omp parallel for
	for (int a = 0; a < (int)particleSz; ++a) {
		ps[a] = (Vec3)poss[a];
	}

	if (_particleSz != particleSz) {
		_particleSz = particleSz;
		SetGLBuf(posBuffer, &ps[0], particleSz, GL_DYNAMIC_DRAW);
		SetGLBuf(colIdBuffer, &colors[0], particleSz);
		SetGLBuf(radBuffer, &radii[0], particleSz);
	}
	else {
		SetGLSubBuf(posBuffer, &ps[0], particleSz);
		SetGLSubBuf(colIdBuffer, &colors[0], particleSz);
		SetGLSubBuf(radBuffer, &radii[0], particleSz);
		UpdateRadBuf();
	}
	SetGLBuf(connBuffer, &conns.ids[0], conns.cnt);
}

void Particles::UpdateColorTex() {
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGBA, GL_FLOAT, _colorPallete);
	glBindTexture(GL_TEXTURE_2D, 0);
	Scene::dirty = true;
}

void Particles::UpdateRadBuf(int i) {
	if (i == -1) {
		std::vector<float> res(particleSz);
#pragma omp parallel for
		for (int a = 0; a < particleSz; ++a) {
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

void Particles::SaveAttrs(const std::string& path) {
	std::string s;
	for (int a = 0; a < attrs.size(); ++a) {
		if (!attrs[a]->readonly) {
			s += attrNms[a] + "#";
			s += attrs[a]->Export();
			s += "\n#";
		}
	}
	IO::WriteFile(path, s);
}

void Particles::LoadAttrs(const std::string& path) {
	auto data = IO::GetText(path);
	auto ss = string_split(data, '#', true);
	auto ssz = ss.size();
	if (!ssz) return;
	int asz = attrs.size();
	for (; asz > 0; --asz) {
		if (!attrs[asz-1]->readonly) {
			RmParam(asz-1);
		}
		else break;
	}
	for (int a = 0; a < ssz/2; ++a) {
		AddParam();
		attrNms[asz++] = ss[a*2];
		auto& at = attrs.back();
		at->Import(ss[a*2+1]);
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
			SetGLSubBuf(posBuffer, (double*)&poss[0], particleSz * 3);
			for (auto& a : attrs) {
				if (a->timed) {
					a->Update();
				}
			}
			if (!!anim.bboxs.size()) {
				memcpy(boundingBox, &anim.bboxs[6*frm], 6*sizeof(double));
				UpdateBBox();
			}
		}
		else anim.Seek(frm);
		for (int i = anim.conns2.size() - 1; i >= 0; --i) {
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
	if (attrs.size() > 48) {
		Debug::Error("Particles::AddParam", "Attribute count limit reached!");
		return;
	}
	attrs.push_back(new paramdata());
	attrNms.push_back("");
	attrNms.rbegin()[1] = "Unnamed " + std::to_string(attrs.size());
}

void Particles::RmParam(int i) {
	delete(attrs[i]);
	attrs.erase(attrs.begin() + i);
	attrNms.erase(attrNms.begin() + i);
}

void Particles::Rebound(glm::dvec3 center) {
	auto co = center - bboxCenter;
	boundingBox[0] += co.x;
	boundingBox[1] += co.x;
	boundingBox[2] += co.y;
	boundingBox[3] += co.y;
	boundingBox[4] += co.z;
	boundingBox[5] += co.z;
	bboxCenter = center;
	if (!!anim.bboxs.size()) memcpy(&anim.bboxs[6*anim.currentFrame], boundingBox, 6*sizeof(double));
	BoundParticles();
	Scene::dirty = true;
}

void Particles::ReboundF(glm::dvec3 center, int f) {
	if (!Particles::anim.bboxs.size()) {
		Particles::anim.FillBBox();
	}
	auto bx = &Particles::anim.bboxs[f*6];
	auto co = center - glm::dvec3((bx[1] + bx[0]),
		(bx[3] + bx[2]),
		(bx[5] + bx[4])) * 0.5;
	bx[0] += co.x;
	bx[1] += co.x;
	bx[2] += co.y;
	bx[3] += co.y;
	bx[4] += co.z;
	bx[5] += co.z;
	BoundParticlesF(f);
}

void Particles::BoundParticles() {
	if (!boxPeriodic) return;
	glm::dvec3 sz (boundingBox[1] - boundingBox[0],
		boundingBox[3] - boundingBox[2],
		boundingBox[5] - boundingBox[4]);
	#pragma omp parallel for
	for (int a = 0; a < particleSz; ++a) {
		glm::dvec3 dp = poss[a] - bboxCenter;
		dp /= sz;
		dp = glm::round(dp);
		dp *= sz;
		poss[a] -= dp;
	}
	bufDirty = true;
	Scene::dirty = true;
}

void Particles::BoundParticlesF(int f) {
	if (!boxPeriodic) return;
	auto bx = &Particles::anim.bboxs[f*6];
	glm::dvec3 sz (bx[1] - bx[0],
		bx[3] - bx[2],
		bx[5] - bx[4]);
	glm::dvec3 isz = glm::dvec3(1, 1, 1) / sz;
	auto& ps = anim.poss[f];
	#pragma omp parallel for
	for (int a = 0; a < particleSz; ++a) {
		glm::dvec3 dp = ps[a] - bboxCenter;
		dp *= isz;
		dp = sz * glm::round(dp);
		ps[a] -= dp;
	}
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