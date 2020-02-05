// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#include "particles.h"
#include "imp/parloader.h"
#include "imp/GenericSSV.h"
#include "Protein.h"
#include "web/anweb.h"
#include "utils/glext.h"

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
		for (size_t a = 0; a < rls; ++a) {
			if (!!(resFlags & (1 << a))) {
				if (rli.name == reslist[a]) goto use;
			}
		}
		continue;
	use:
		uint first = rli.residues[0].offset;
		uint last = rli.residues.back().offset + rli.residues.back().cnt;
		for (uint a = first; a < last; ++a) {
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

bool Particles::empty;

uint Particles::residueListSz;
uint Particles::particleSz;
uint Particles::maxParticleSz;

std::string Particles::cfgFile, Particles::trjFile;

glm::dvec3* Particles::poss, *Particles::vels;

std::vector<ResidueList> Particles::residueLists;
std::vector<std::string> Particles::reslist;

std::vector<char> Particles::names, Particles::resNames;
std::vector<uint16_t> Particles::types;
std::vector<byte> Particles::colors;
std::vector<float> Particles::radii;
std::vector<float> Particles::radiiscl;
std::vector<bool> Particles::visii;
std::vector<Int2> Particles::ress;
Particles::conninfo Particles::conns;

bool Particles::bufDirty = false, Particles::visDirty = false, Particles::palleteDirty = false;

std::vector<Particles::attrdata*> Particles::attrs;
std::vector<std::string> Particles::attrNms;
uint Particles::readonlyAttrCnt;

std::vector<Particles::conninfo> Particles::particles_Conn2;

Particles::animdata Particles::anim = {};

double Particles::boundingBox[] = {};
glm::dvec3 Particles::bboxCenter;
bool Particles::boxPeriodic = false;

std::vector<Particles::DefColor> Particles::defColors;
std::vector<std::pair<ushort, Vec3>> Particles::colorPallete;
Vec4 Particles::_colorPallete[256];
std::vector<Particles::SpecificColor> Particles::colorOverrides;
GLuint Particles::colorPalleteTex;

GLuint Particles::posBuffer;
GLuint Particles::connBuffer;
GLuint Particles::colIdBuffer;
GLuint Particles::radBuffer;
GLuint Particles::posTexBuffer, Particles::connTexBuffer, Particles::colorIdTexBuffer, Particles::radTexBuffer;

void Particles::Init() {
	glGenBuffers(1, &posBuffer);
	glGenBuffers(1, &connBuffer);
	glGenBuffers(1, &colIdBuffer);
	glGenBuffers(1, &radBuffer);

	glGenTextures(1, &colorPalleteTex);
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_FLOAT, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	attrNms.reserve(50); //reallocation will break something, temporary for now
	attrNms.push_back("");

	empty = true;
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
		readonlyAttrCnt = 0;
		residueListSz = particleSz = Particles::conns.cnt = 0;
		
		anim.Clear();
		Protein::Clear();

		VisSystem::radii.clear();
	}
	empty = true;
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
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, colIdBuffer);

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

	if (maxParticleSz < particleSz) {
		maxParticleSz = particleSz;
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
		for (int a = 0; a < (int)particleSz; ++a) {
			res[a] = visii[a] ? std::max(radii[a], 0.001f)*radiiscl[a] : -1;
		}
		SetGLSubBuf(radBuffer, &res[0], particleSz);
	}
	else {
		glBindBuffer(GL_ARRAY_BUFFER, radBuffer);
		float vl = visii[i] ? std::max(radii[i], 0.001f)*radiiscl[i] : -1;
		glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(float), sizeof(float), &vl);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	Scene::dirty = true;
}

void Particles::SaveAttrs(const std::string& path) {
	std::ofstream strm(path, std::ios::binary);
	for (size_t a = 0; a < attrs.size(); ++a) {
		if (!attrs[a]->readonly) {
			strm.write(attrNms[a].data(), attrNms[a].size() + 1);
			attrs[a]->Export(strm);
		}
	}
}

void Particles::LoadAttrs(const std::string& path) {
	auto data = IO::GetText(path);
	auto asz = attrs.size();
	for (; asz > 0; --asz) {
		if (!attrs[asz-1]->readonly) {
			RmAttr(asz-1);
		}
		else break;
	}
	std::ifstream strm(path, std::ios::binary);
	while (strm) {
		char cc[100];
		strm.getline(cc, 100, 0);
		if (!cc[0]) break;
		AddAttr();
		attrNms[asz++] = std::string(cc);
		attrs.back()->Import(strm);
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
		if (anim.currentFrame != ~0U) {
			anim.Seek(frm);
			if (anim.status[frm] != animdata::FRAME_STATUS::LOADED) return;
			auto& aps = anim.poss[anim.currentFrame];
			particleSz = aps.size();
			poss = &aps[0];
			SetGLSubBuf(posBuffer, (double*)&poss[0], particleSz * 3);
			for (auto& a : attrs) {
				if (a->timed) {
					a->Update();
				}
			}
			if (!!anim.bboxs.size()) {
				memcpy(boundingBox, &anim.bboxs[frm][0], 6*sizeof(double));
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

void Particles::AddAttr(bool readonly) {
	if (attrs.size() > 48) {
		Debug::Error("Particles::AddAttr", "attrdata count limit reached!");
		return;
	}
	attrs.push_back(new attrdata(readonly));
	attrNms.push_back("");
	attrNms.rbegin()[1] = "Unnamed " + std::to_string(attrs.size());
}

void Particles::RmAttr(int i) {
	delete(attrs[i]);
	attrs.erase(attrs.begin() + i);
	attrNms.erase(attrNms.begin() + i);
}

void Particles::UpdateAttrs() {
	for (auto& a : attrs) {
		a->Update();
	}
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
	if (!Particles::anim.bboxs.size()) {
		Particles::anim.FillBBox();
	}
	memcpy(&anim.bboxs[anim.currentFrame][0], boundingBox, 6*sizeof(double));
	anim.bboxState[anim.currentFrame] = animdata::BBOX_STATE::CHANGED;
	BoundParticles();
	Scene::dirty = true;
}

void Particles::ReboundF(glm::dvec3 center, int f) {
	if (!Particles::anim.bboxs.size()) {
		Particles::anim.FillBBox();
	}
	auto& bx = Particles::anim.bboxs[f];
	auto co = center - glm::dvec3((bx[1] + bx[0]),
		(bx[3] + bx[2]),
		(bx[5] + bx[4])) * 0.5;
	bx[0] += co.x;
	bx[1] += co.x;
	bx[2] += co.y;
	bx[3] += co.y;
	bx[4] += co.z;
	bx[5] += co.z;
	anim.bboxState[f] = animdata::BBOX_STATE::CHANGED;
	BoundParticlesF(f);
}

void Particles::BoundParticles() {
	if (!boxPeriodic) return;
	glm::dvec3 sz (boundingBox[1] - boundingBox[0],
		boundingBox[3] - boundingBox[2],
		boundingBox[5] - boundingBox[4]);
#pragma omp parallel for
	for (int a = 0; a < (int)particleSz; ++a) {
		glm::dvec3 dp = poss[a] - bboxCenter;
		dp /= sz;
		dp = glm::round(dp);
		dp *= sz;
		poss[a] -= dp;
	}
	if (!Particles::anim.bboxs.size()) {
		Particles::anim.FillBBox();
	}
	anim.bboxState[anim.currentFrame] = animdata::BBOX_STATE::PERIODIC;
	bufDirty = true;
	Scene::dirty = true;
}

void Particles::BoundParticlesF(int f) {
	if (!boxPeriodic) return;
	if (!Particles::anim.bboxs.size()) {
		Particles::anim.FillBBox();
	}
	auto& bx = Particles::anim.bboxs[f];
	glm::dvec3 sz (bx[1] - bx[0],
		bx[3] - bx[2],
		bx[5] - bx[4]);
	auto co = glm::dvec3((bx[1] + bx[0]),
		(bx[3] + bx[2]),
		(bx[5] + bx[4])) * 0.5;
	glm::dvec3 isz = glm::dvec3(1, 1, 1) / sz;
	auto& ps = anim.poss[f];
#pragma omp parallel for
	for (int a = 0; a < (uint)particleSz; ++a) {
		glm::dvec3 dp = ps[a] - co;
		dp *= isz;
		dp = sz * glm::round(dp);
		ps[a] -= dp;
	}
	anim.bboxState[f] = animdata::BBOX_STATE::PERIODIC;
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
							Engine::stateLock.unlock();
							while (ParLoader::busy){}
							Engine::WaitForLockValue();
							Engine::stateLock.lock();
						}
						else if (!!particleSz) {
							if (n3.name == "trajectory" && n3.value != "") {
								ParLoader::directLoad = true;
								if (n3.params["relative"] == "1")
									ParLoader::OnOpenFile(std::vector<std::string>{ s + n3.value });
								else
									ParLoader::OnOpenFile(std::vector<std::string>{ n3.value });
								//
								Engine::stateLock.unlock();
								while (ParLoader::busy){}
								Engine::WaitForLockValue();
								Engine::stateLock.lock();
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