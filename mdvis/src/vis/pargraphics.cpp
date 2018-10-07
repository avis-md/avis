#include "pargraphics.h"
#include "md/ParMenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "vis/system.h"
#include "res/resdata.h"
#include "ui/icons.h"
#include "ui/popups.h"
#include "ui/ui_ext.h"
#include "ui/localizer.h"
#include "utils/effects.h"
#include "web/anweb.h"
#include "shadows.h"
#include "hdr.h"
#include "ocl/raytracer.h"
#include "res/shddata.h"

#define SCL_MIN -7.0f
#define SCL_MAX 2.0f


Texture ParGraphics::bg, ParGraphics::splash, ParGraphics::logo;
GLuint ParGraphics::refl, ParGraphics::reflE;
float ParGraphics::reflStr = 2, ParGraphics::reflStrDecay = 2, ParGraphics::specStr = 0.2f;
Vec4 ParGraphics::bgCol = Vec4(1, 1, 1, 1);

int ParGraphics::reflId = 0, ParGraphics::_reflId = -1;
std::vector<std::string> ParGraphics::reflNms;
Popups::DropdownItem ParGraphics::reflItms((uint*)&reflId, nullptr);

bool ParGraphics::useGradCol = false;
uint ParGraphics::gradColParam;
Vec4 ParGraphics::gradCols[] = { blue(), green(), red() };
bool ParGraphics::useConCol, ParGraphics::useConGradCol;
Vec4 ParGraphics::conCol = white();

PROGDEF(ParGraphics::reflProg);
PROGDEF(ParGraphics::reflCProg);
PROGDEF(ParGraphics::parProg);
PROGDEF(ParGraphics::parConProg);
PROGDEF(ParGraphics::parConLineProg);
PROGDEF(ParGraphics::selHlProg);
PROGDEF(ParGraphics::colProg);

std::vector<uint> ParGraphics::hlIds, ParGraphics::selIds;
std::vector<std::pair<uint, std::pair<uint, byte>>> ParGraphics::drawLists, ParGraphics::drawListsB;

uint ParGraphics::usePBR = 1;
std::string ParGraphics::_usePBRNms[] = {"", "", "\0"};
const Popups::DropdownItem ParGraphics::_usePBRItems = Popups::DropdownItem(&ParGraphics::usePBR, (std::string*)&ParGraphics::_usePBRNms[0]);

Vec3 ParGraphics::rotCenter = Vec3();
uint ParGraphics::rotCenterTrackId = -1;
float ParGraphics::rotW = 0, ParGraphics::rotZ = 0;
float ParGraphics::rotWs = 0, ParGraphics::rotZs = 0;
float ParGraphics::rotScale = 0;

Mesh* ParGraphics::arrowMesh;

ParGraphics::CLIPPING ParGraphics::clippingType, ParGraphics::_clippingType;
ParGraphics::ClipPlane ParGraphics::clipPlane = {};
ParGraphics::ClipCube ParGraphics::clipCube = {};
GLuint ParGraphics::clipUbo;
Vec4 ParGraphics::clippingPlanes[] = {};

float ParGraphics::zoomFade = 0;

Vec3 ParGraphics::scrX, ParGraphics::scrY, ParGraphics::scrZ;

bool ParGraphics::dragging = false;
byte ParGraphics::dragMode = 0;

bool ParGraphics::animate = false, ParGraphics::seek = false;
float ParGraphics::animOff;
int ParGraphics::animTarFps = 30;
bool ParGraphics::tfboDirty = true;
Mat4x4 ParGraphics::lastMVP;

//---------------- effects vars -------------------

bool ParGraphics::Eff::expanded;
bool ParGraphics::Eff::showSSAO, ParGraphics::Eff::showGlow;
bool ParGraphics::Eff::useSSAO, ParGraphics::Eff::useGlow;

float ParGraphics::Eff::ssaoRad, ParGraphics::Eff::ssaoStr, ParGraphics::Eff::ssaoBlur;
int ParGraphics::Eff::ssaoSamples;

float ParGraphics::Eff::glowThres, ParGraphics::Eff::glowRad, ParGraphics::Eff::glowStr;

void ParGraphics::Eff::Apply() {
	auto cam = ChokoLait::mainCamera().get();
	byte cnt = 0;
	if (tfboDirty) {
		//cnt += Effects::Blur(cam->blitFbos[0], cam->blitFbos[1], cam->blitTexs[0], cam->blitTexs[1], _rad*20, Display::width, Display::height);
		if (useSSAO) cnt += Effects::SSAO(cam->blitFbos[0], cam->blitFbos[1], cam->blitFbos[2], cam->blitTexs[0], cam->blitTexs[1], cam->blitTexs[2],
			cam->texs.normTex, cam->texs.depthTex, ssaoStr, ssaoSamples, ssaoRad, ssaoBlur, Display::width, Display::height);

		if ((cnt % 2) == 1) {
			std::swap(cam->blitFbos[0], cam->blitFbos[1]);
			std::swap(cam->blitTexs[0], cam->blitTexs[1]);
			cnt = 0;
		}
		if (AnWeb::drawFull) cnt += Effects::Blur(cam->blitFbos[0], cam->blitFbos[1], cam->blitTexs[0], cam->blitTexs[1], 1.0f, Display::width, Display::height);
	}
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cam->target);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, cam->blitFbos[cnt % 2]);
	
	glViewport(0, 0, Display::frameWidth, Display::frameHeight);

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, Display::width, Display::height, 0, 0, Display::frameWidth, Display::frameHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

float ParGraphics::Eff::DrawMenu(float off) {
	auto& expandPos = ParMenu::expandPos;
	
	UI::Label(expandPos - 148, off, 12, _("Effects"), white());

	off += 17;
	UI::Quad(expandPos - 148, off - 1, 147, 17 * 5 + 2, white(0.9f, 0.1f));
	UI::Label(expandPos - 146, off, 12, _("Ambient Occlusion"), white());
	useSSAO = Engine::Toggle(expandPos - 19, off, 16, Icons::checkbox, useSSAO, white(), ORIENT_HORIZONTAL);
	ssaoSamples = (int)UI2::Slider(expandPos - 147, off + 17, 147, _("Samples"), 5, 100, (float)ssaoSamples, std::to_string(ssaoSamples));
	ssaoSamples = Clamp(ssaoSamples, 10, 100);
	ssaoRad = UI2::Slider(expandPos - 147, off + 17 * 2, 147, _("Radius"), 0.001f, 0.05f, ssaoRad);
	ssaoStr = UI2::Slider(expandPos - 147, off + 17 * 3, 147, _("Strength"), 0, 3, ssaoStr);
	ssaoBlur = UI2::Slider(expandPos - 147, off + 17 * 4, 147, _("Blur"), 0, 40, ssaoBlur);
	return off + 17 * 5 + 1;
}

void ParGraphics::Eff::Serialize(XmlNode* nd) {
	auto n = nd->addchild("SSAO");
#define SVS(nm, vl) n->addchild(#nm, vl)
#define SV(nm, vl) SVS(nm, std::to_string(vl))
	SVS(enabled, useSSAO? "1" : "0");
	SV(samples, ssaoSamples); SV(radius, ssaoRad);
	SV(strength, ssaoStr); SV(blur, ssaoBlur);
#undef SVS
#undef SV
}

void ParGraphics::Eff::Deserialize(XmlNode* nd) {
#define GT(nm, vl) if (n.name == #nm) vl = TryParse(n.value, vl)
#define GTV(nm, vl) if (n.name == #nm) Xml::ToVec(&n, vl)
	for (auto& n1 : nd->children) {
		if (n1.name == "SSAO") {
			for (auto& n : n1.children) {
				if (n.name == "enabled") useSSAO = (n.value == "1");
				else GT(samples, ssaoSamples);
				else GT(radies, ssaoRad);
				else GT(strength, ssaoStr);
				else GT(blur, ssaoBlur);
			}
		}
	}
#undef GT
#undef GTV
}

void ParGraphics::Init() {
	const GLuint _clipBindId = 11;

	_usePBRNms[0] = _("Classic");
	_usePBRNms[1] = _("PBR");
	std::ifstream strm(IO::path + "backgrounds/default");
	strm >> reflId;
	strm.close();
	std::vector<std::string> fds;
	IO::GetFolders(IO::path + "backgrounds/", &fds);
	for (auto& fd : fds) {
		if (IO::HasFile(IO::path + "backgrounds/" + fd + "/diffuse.hdr") &&
			IO::HasFile(IO::path + "backgrounds/" + fd + "/specular.hdr")) {
			reflNms.push_back(fd);
		}
	}
	reflNms.push_back("\0");
	reflItms.list = &reflNms[0];

	bg = Texture(IO::path + "res/bg.jpg", false, TEX_FILTER_BILINEAR, 1, TEX_WRAP_CLAMP);
	splash = Texture(IO::path + "res/bg_splash.jpg", false, TEX_FILTER_BILINEAR, 1, TEX_WRAP_CLAMP);
	GLuint mv;
	Shader::LoadShader(GL_VERTEX_SHADER, glsl::minVert, mv);
	reflProg = Shader::FromF(mv, glsl::reflFrag);
#define LC(nm) reflProgLocs[i++] = glGetUniformLocation(reflProg, #nm)
	uint i = 0;
	LC(_IP); LC(screenSize); LC(inColor); LC(inNormal);
	LC(inEmit); LC(inDepth); LC(inSky); LC(inSkyE);
	LC(skyStrength); LC(skyStrDecay); LC(specStr); LC(bgCol);
#undef LC
	
	reflCProg = Shader::FromF(mv, glsl::reflFragC);
#define LC(nm) reflCProgLocs[i++] = glGetUniformLocation(reflCProg, #nm)
	i = 0;
	LC(_IP); LC(screenSize); LC(inColor);
	LC(inNormal); LC(inDepth); LC(skyStrength);
	LC(skyStrDecay); LC(specStr); LC(bgCol);
#undef LC
	
	parProg = Shader::FromVF(IO::GetText(IO::path + "parV.txt"), IO::GetText(IO::path + "parF.txt"));
#define LC(nm) parProgLocs[i++] = glGetUniformLocation(parProg, #nm)
	i = 0;
	LC(_MV); LC(_P); LC(camPos);
	LC(camFwd); LC(orthoSz); LC(screenSize);
	LC(radTex); LC(radScl); LC(id2col); LC(colList);
	LC(gradCols); LC(colUseGrad); LC(spriteScl);
	auto bid = glGetUniformBlockIndex(parProg, "clipping");
	glUniformBlockBinding(parProg, bid, _clipBindId);
#undef LC

	parConProg = Shader::FromVF(IO::GetText(IO::path + "parConV.txt"), IO::GetText(IO::path + "parConF.txt"));
#define LC(nm) parConProgLocs[i++] = glGetUniformLocation(parConProg, #nm)
	i = 0;
	LC(_MV); LC(_P); LC(camPos); LC(camFwd);
	LC(screenSize); LC(posTex); LC(connTex);
	LC(id2); LC(radScl); LC(orthoSz);
	LC(id2col); LC(colList); 
	LC(gradCols); LC(colUseGrad);
	LC(usegrad); LC(onecol); LC(spriteScl);
	bid = glGetUniformBlockIndex(parConProg, "clipping");
	glUniformBlockBinding(parConProg, bid, _clipBindId);
#undef LC

	parConLineProg = Shader::FromVF(IO::GetText(IO::path + "parConV_line.txt"), IO::GetText(IO::path + "parConF_line.txt"));
#define LC(nm) parConLineProgLocs[i++] = glGetUniformLocation(parConLineProg, #nm)
	i = 0;
	LC(_MV); LC(_P); LC(posTex);
	LC(connTex), LC(rad); LC(id2);
#undef LC

	selHlProg = Shader::FromF(mv, IO::GetText(IO::path + "selectorFrag.txt"));
#define LC(nm) selHlProgLocs[i++] = glGetUniformLocation(selHlProg, #nm)
	i = 0;
	LC(screenSize); LC(myId); LC(idTex); LC(hlCol);
#undef LC

	colProg = Shader::FromF(mv, glsl::colererFrag);
#define LC(nm) colProgLocs[i++] = glGetUniformLocation(colProg, #nm)
	i = 0;
	LC(idTex); LC(spTex); LC(screenSize);
	LC(id2col); LC(colList); LC(usegrad);
	LC(gradcols); LC(doid); LC(tint);
#undef LC

	glDeleteShader(mv);

	clippingPlanes[0] = Vec4(0, 1, 0, 0);
	glGenBuffers(1, &clipUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, clipUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Vec4)*6, &clippingPlanes[0][0], GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, _clipBindId, clipUbo);

	hlIds.resize(1);
	ChokoLait::mainCamera->onBlit = Reblit;
	//ChokoLait::mainCamera->useGBuffer2 = true;
	//ChokoLait::mainCamera->quality2 = 0.5f;

	Eff::ssaoSamples = 30;
	Eff::ssaoRad = 0.015f;
	Eff::ssaoStr = 1;
	Eff::ssaoBlur = 0;

	InitClippingMesh();
}

void ParGraphics::InitClippingMesh() {
	Vec3 pts[14];
	for (int a = 0; a < 4; a++) {
		pts[a] = Vec3(cosf(a*PI/2), sinf(a*PI/2), 0) * 0.05f;
		pts[a+4] = pts[a];
		pts[a+8] = pts[a]*3.0f;
		pts[a+8].z = pts[a+4].z = 0.5f;
	}
	pts[13] = Vec3(0, 0, 0.6f);
	int ids[36] = {
		0,4,1,4,5,1,	1,5,2,5,6,2,
		2,6,3,6,7,3,	3,7,0,7,4,0,
		8,12,9, 9,12,10, 10,12,11, 11,12,9
	};
	arrowMesh = new Mesh(13, pts, 0, 36, ids);
}

void ParGraphics::UpdateDrawLists() {
	drawLists.clear();
	drawListsB.clear();
	int di = -1, di2 = -1;
	byte dt;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& r = Particles::residueLists[i];

		if (r.drawType == 255 || !r.visibleAll) {
			for (uint j = 0; j < r.residueSz; j++) {
				auto rr = r.residues[j];
				if ((di == -1) && rr.visible) {
					di = rr.offset;
					di2 = rr.offset_b;
					dt = rr.drawType;
				}
				else if ((di > -1) && ((!rr.visible) || (dt != rr.drawType))) {
					if (!!(dt & 0x0f) && (dt != 0x11)) {
						drawLists.push_back(std::pair<uint, std::pair<uint, byte>>(di, std::pair<uint, byte>(rr.offset - di, (dt == 0x01) ? 0x0f : (dt & 0x0f))));
					}
					auto bcnt = rr.offset_b - di2;
					if (!!bcnt && !!(dt >> 4)) drawListsB.push_back(std::pair<uint, std::pair<uint, byte>>(di2, std::pair<uint, byte>(bcnt, dt >> 4)));
					if (!rr.visible) di = -1;
					else {
						di = rr.offset;
						di2 = rr.offset_b;
						dt = rr.drawType;
					}
				}
			}
		}
		else {
			if ((di == -1) && r.visible) {
				di = r.residues[0].offset;
				di2 = r.residues[0].offset_b;
				dt = r.drawType;
			}
			else if ((di > -1) && ((!r.visible) || (dt != r.drawType))) {
				if (!!(dt & 0x0f) && (dt != 0x11)) {
					drawLists.push_back(std::pair<uint, std::pair<uint, byte>>(di, std::pair<uint, byte>(r.residues[0].offset - di, (dt == 0x01) ? 0x0f : (dt & 0x0f))));
				}
				auto bcnt = r.residues[0].offset_b - di2;
				if (!!bcnt && !!(dt >> 4)) drawListsB.push_back(std::pair<uint, std::pair<uint, byte>>(di2, std::pair<uint, byte>(bcnt, dt >> 4)));
				if (!r.visible) di = -1;
				else {
					di = r.residues[0].offset;
					di2 = r.residues[0].offset_b;
					dt = r.drawType;
				}
			}
		}
	}
	if (di > -1) {
		auto& rl = Particles::residueLists[Particles::residueListSz - 1];
		auto& rs = rl.residues[rl.residueSz-1];
		if (!!(dt & 0x0f) && (dt != 0x11)) {
			drawLists.push_back(std::pair<uint, std::pair<uint, byte>>(di, std::pair<uint, byte>(rs.offset + rs.cnt - di, (dt == 0x01) ? 0x0f : (dt & 0x0f))));
		}
		auto bcnt = rs.offset_b + rs.cnt_b - di2;
		if (!!bcnt && !!(dt >> 4)) drawListsB.push_back(std::pair<uint, std::pair<uint, byte>>(di2, std::pair<uint, byte>(bcnt, dt >> 4)));
	}
	Scene::dirty = true;
}

void ParGraphics::FillRad(byte* rads) {
	for (auto& p : drawLists) {
		float ml = 1;
		switch (p.second.second) {
		case 1:
			for (uint a = p.first; a < p.first + p.second.first; a++) {
				rads[a] = 255;
			}
			continue;
		case 0x0f:
			continue;
		case 2:
			ml = 0.2f; break;
		default: break;
		}
		for (uint a = p.first; a < p.first + p.second.first; a++) {
			rads[a] = (byte)(min(0.1f * ml * Particles::particles_Rad[a], 0.2f) * 255 / 0.2f);
		}
	}
}

void ParGraphics::Update() {
	if (!Particles::particleSz)
		Scene::dirty = true;
	else if (animate && !seek) {
		if (!animTarFps) {
			Particles::IncFrame(true);
		}
		else {
			auto af = Particles::anim.currentFrame;
			auto dt = Time::delta + animOff;
			auto df = dt * animTarFps;
			auto dfi = (uint)floor(df);
			animOff = (df - dfi) / animTarFps;
			af += dfi;
			while (af >= Particles::anim.frameCount) af -= Particles::anim.frameCount;
			Particles::SetFrame(af);
		}
		Scene::dirty = true;
	}
	if (!!Particles::particleSz && !UI::editingText && !UI::_layerMax) {
		rotW = Clamp<float>(rotW, -90, 90);
		rotZ = Repeat<float>(rotZ, 0, 360);
		rotScale = Clamp(rotScale, SCL_MIN, SCL_MAX);

		float s0 = rotScale;
		float rz0 = rotZ;
		float rw0 = rotW;
		Vec3 center0 = rotCenter;

		if (Input::mouse0) {
			if (Input::mouse0State == MOUSE_DOWN) {
				if (Input::KeyHold(Key_LeftShift)) dragMode = 2;
				else dragMode = 0;
			}
			else if (Input::mouse0State == MOUSE_HOLD && !dragging && VisSystem::InMainWin(Input::mouseDownPos)) {
				dragging = true;
				if (!ChokoLait::mainCamera->applyGBuffer2) Scene::dirty = true;
				ChokoLait::mainCamera->applyGBuffer2 = true;
			}
			else if (dragging) {
				if ((VisSystem::mouseMode == VIS_MOUSE_MODE::ROTATE) && (dragMode != 2)) {
					rotW = rotWs;
					rotZ = rotZs;
					rotW += 180 * Input::mouseDelta.y / Display::height;
					rotZ += 180 * Input::mouseDelta.x / Display::width;
					rotW = Clamp<float>(rotW, -90, 90);
					rotZ = Repeat<float>(rotZ, 0, 360);
					rotWs = rotW;
					rotZs = rotZ;
					if (Input::KeyHold(Key_LeftShift)) {
						const float dth = 22.5f;
						rotW = dth * round(rotW / dth);
						rotZ = dth * round(rotZ / dth);
					}
				}
				else if ((VisSystem::mouseMode == VIS_MOUSE_MODE::PAN) || (((VisSystem::mouseMode == VIS_MOUSE_MODE::ROTATE) && (dragMode == 2)))) {
					rotCenter -= 5.0f * scrX * (Input::mouseDelta.x / Display::width);
					rotCenter += 5.0f * scrY * (Input::mouseDelta.y / Display::height);
				}
			}
		}
		else {
			if (dragging) {
				dragging = false;
				rotWs = rotW;
				rotZs = rotZ;
			}
			if (ChokoLait::mainCamera->applyGBuffer2) {
				ChokoLait::mainCamera->applyGBuffer2 = false;
				Scene::dirty = true;
			}
			if (Input::mouseScroll != 0 && VisSystem::InMainWin(Input::mousePos)) {
				if (Input::KeyHold(Key_LeftShift)) {
					rotCenter -= 0.1f * scrZ * Input::mouseScroll / powf(2, rotScale);
				}
				else {
					rotScale += 0.05f * Input::mouseScroll;
					rotScale = Clamp(rotScale, SCL_MIN, SCL_MAX);
					zoomFade = 2;
				}
				ChokoLait::mainCamera->applyGBuffer2 = true;
			}
			else {
				if (Input::KeyDown(Key_Escape)) {
					VisSystem::mouseMode = VIS_MOUSE_MODE::ROTATE;
				}
			}
		}
		if (s0 != rotScale || rz0 != rotZ || rw0 != rotW || center0 != rotCenter) Scene::dirty = true;

	}
	if (reflId != _reflId) {
		_reflId = reflId;
		auto pth = IO::path + "backgrounds/" + reflNms[reflId] + "/";
		if (!!refl) {
			glDeleteTextures(1, &refl);
			glDeleteTextures(1, &reflE);
		}
		uint _w, _h;
		byte* d = hdr::read_hdr((pth + "specular.hdr").c_str(), &_w, &_h);
		float* dv = new float[_w*_h * 3];
		if (d) {
			hdr::to_float(d, _w, _h, dv);
			//byte* d = Texture::LoadPixels(IO::path + "res/?.png", chn, _w, _h);
			glGenTextures(1, &refl);
			glBindTexture(GL_TEXTURE_2D, refl);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _w, _h, 0, GL_RGB, GL_FLOAT, dv);
			SetTexParams<>(0, GL_REPEAT, GL_MIRRORED_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete[](d);
		}
		delete[](dv);
		d = hdr::read_hdr((pth + "diffuse.hdr").c_str(), &_w, &_h);
		if (d) {
			dv = new float[_w*_h * 3];
			hdr::to_float(d, _w, _h, dv);
			delete[](d);
			glGenTextures(1, &reflE);
			glBindTexture(GL_TEXTURE_2D, reflE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _w, _h, 0, GL_RGB, GL_FLOAT, dv);
			SetTexParams<>(0, GL_REPEAT, GL_MIRRORED_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete[](dv);
		}
	}
}

#define TTF(f, t) *(Vec3*)&f = t
void ParGraphics::UpdateClipping() {
	auto mv = MVP::modelview();
	switch (clippingType) {
	case CLIPPING::NONE:
		for (int a = 0; a < 6; a++) {
			clippingPlanes[a] = Vec4();
		}
		break;
	case CLIPPING::PLANE:
	{
		auto nl = glm::length(clipPlane.norm);
		if (nl > 0)
			clipPlane.norm /= nl;
		Vec4 cents[2] = {}, dirs[2] = {};
		TTF(dirs[0], clipPlane.norm);
		TTF(dirs[1], -clipPlane.norm);
		TTF(cents[0], clipPlane.center + clipPlane.norm * clipPlane.size * 0.5f);
		TTF(cents[1], clipPlane.center - clipPlane.norm * clipPlane.size * 0.5f);
		cents[0].w = cents[1].w = 1;
		dirs[0] = glm::normalize(mv * dirs[0]);
		dirs[1] = glm::normalize(mv * dirs[1]);
		clippingPlanes[0] = dirs[0];
		clippingPlanes[1] = dirs[1];
		for (int a = 0; a < 2; a++) {
			cents[a] = mv * cents[a];
			cents[a] /= cents[a].w;
			clippingPlanes[a].w = glm::dot((Vec3)cents[a], (Vec3)clippingPlanes[a]);
		}
		for (int a = 2; a < 6; a++) {
			clippingPlanes[a] = Vec4();
		}
		break;
	}
	case CLIPPING::CUBE:
	{
		auto cs = Vec4(clipCube.center, 1);
		Vec4 cents[6], dirs[3];
		dirs[0] = Vec4(1, 0, 0, 0);
		dirs[1] = Vec4(0, 1, 0, 0);
		dirs[2] = Vec4(0, 0, 1, 0);
		cents[0] = cs - dirs[0] * clipCube.size.x * 0.5f;
		cents[1] = cs + dirs[0] * clipCube.size.x * 0.5f;
		cents[2] = cs - dirs[1] * clipCube.size.y * 0.5f;
		cents[3] = cs + dirs[1] * clipCube.size.y * 0.5f;
		cents[4] = cs - dirs[2] * clipCube.size.z * 0.5f;
		cents[5] = cs + dirs[2] * clipCube.size.z * 0.5f;
		for (int a = 0; a < 3; a++) {
			dirs[a] = glm::normalize(mv * dirs[a]);
		}
		clippingPlanes[0] = -dirs[0];
		clippingPlanes[1] = dirs[0];
		clippingPlanes[2] = -dirs[1];
		clippingPlanes[3] = dirs[1];
		clippingPlanes[4] = -dirs[2];
		clippingPlanes[5] = dirs[2];
		for (int a = 0; a < 6; a++) {
			cents[a] = mv * cents[a];
			cents[a] /= cents[a].w;
			clippingPlanes[a].w = glm::dot((Vec3)cents[a], (Vec3)clippingPlanes[a]);
		}
		break;
	}
	}

	glBindBuffer(GL_UNIFORM_BUFFER, clipUbo);
	GLvoid* p = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(p, &clippingPlanes[0][0], sizeof(Vec4)*6);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	Scene::dirty = true;
}

void ParGraphics::Rerender(Vec3 _cpos, Vec3 _cfwd, float _w, float _h) {
	if (!!Particles::particleSz) {
		float csz = cos(-rotZ*deg2rad);
		float snz = sin(-rotZ*deg2rad);
		float csw = cos(-rotW*deg2rad);
		float snw = sin(-rotW*deg2rad);
		Mat4x4 mMatrix = Mat4x4(1, 0, 0, 0, 0, csw, snw, 0, 0, -snw, csw, 0, 0, 0, 0, 1) * Mat4x4(csz, 0, -snz, 0, 0, 1, 0, 0, snz, 0, csz, 0, 0, 0, 0, 1);
		MVP::Mul(mMatrix);
		float s = pow(2.0f, rotScale);
		MVP::Scale(s, s, s);
		if (rotCenterTrackId < ~0) {
			rotCenter = Particles::particles_Pos[rotCenterTrackId];
		}
		MVP::Translate(-rotCenter.x, -rotCenter.y, -rotCenter.z);
		auto _mv = MVP::modelview();
		auto _p = MVP::projection();

		UpdateClipping();

		if (!Shadows::isPass) {
			lastMVP = _p * _mv;
			auto imvp = glm::inverse(lastMVP);
			scrX = Vec3(imvp * Vec4(1, 0, 0, 0));
			scrY = Vec3(imvp * Vec4(0, 1, 0, 0));
			scrZ = glm::normalize(glm::cross(scrY, scrX));
		}

		auto ql = ChokoLait::mainCamera->quality;
		auto spriteScl = ChokoLait::mainCamera->scale;

		float osz = -1;
		if (ChokoLait::mainCamera->ortographic) {
			Vec4 p1 = _p * Vec4(-1, 1, -1, 1);
			p1 /= p1.w;
			Vec4 p2 = _p * Vec4(1, 1, -1, 1);
			p2 /= p2.w;
			osz = glm::length(p2 - p1)/2;
		}

		if (!RayTracer::resTex) {
			glUseProgram(parProg);
			glUniformMatrix4fv(parProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
			glUniformMatrix4fv(parProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
			glUniform3f(parProgLocs[2], _cpos.x, _cpos.y, _cpos.z);
			glUniform3f(parProgLocs[3], _cfwd.x, _cfwd.y, _cfwd.z);
			glUniform1f(parProgLocs[4], osz);
			glUniform2f(parProgLocs[5], _w * ql, _h * ql);
			glUniform1i(parProgLocs[6], 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_BUFFER, Particles::radTexBuffer);
			glUniform1i(parProgLocs[8], 2);
			glActiveTexture(GL_TEXTURE2);
			if (!useGradCol) {
				glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
				glUniform1i(parProgLocs[9], 3);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);
			}
			else {
				glBindTexture(GL_TEXTURE_BUFFER, Particles::particles_Params[gradColParam]->texBuf);
				glUniform4fv(parProgLocs[10], 3, &gradCols[0][0]);
			}
			glUniform1i(parProgLocs[11], useGradCol);
			glUniform1f(parProgLocs[12], spriteScl);

			glBindVertexArray(Particles::posVao);
			for (auto& p : drawLists) {
				if (p.second.second == 1) glUniform1f(parProgLocs[7], -1);
				else if (p.second.second == 0x0f) glUniform1f(parProgLocs[7], 0);
				else if (p.second.second == 2) glUniform1f(parProgLocs[7], 0.2f);
				else glUniform1f(parProgLocs[7], 1);
				glDrawArrays(GL_POINTS, p.first, p.second.first);
			}

			auto& con = Particles::particles_Conn;
			glBindVertexArray(Camera::emptyVao);
			for (auto& p : drawListsB) {
				byte& tp = p.second.second;
				if (tp == 1) {
					glUseProgram(parConLineProg);
					glUniformMatrix4fv(parConLineProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
					glUniformMatrix4fv(parConLineProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
					glUniform1i(parConLineProgLocs[2], 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
					glUniform1i(parConLineProgLocs[3], 2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::connTexBuffer);
					glUniform2f(parConLineProgLocs[4], con.line_sc / Display::width, con.line_sc / Display::height);
					glUniform1ui(parConLineProgLocs[5], 1);
					glDrawArrays(GL_TRIANGLES, p.first * 6, p.second.first * 6);
				}
				else {
					glUseProgram(parConProg);
					glUniformMatrix4fv(parConProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
					glUniformMatrix4fv(parConProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
					glUniform3f(parConProgLocs[2], _cpos.x, _cpos.y, _cpos.z);
					glUniform3f(parConProgLocs[3], _cfwd.x, _cfwd.y, _cfwd.z);
					glUniform2f(parConProgLocs[4], _w * ql, _h * ql);
					glUniform1i(parConProgLocs[5], 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
					glUniform1i(parConProgLocs[6], 2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::connTexBuffer);
					glUniform1ui(parConProgLocs[7], 1);
					glUniform1f(parConProgLocs[8], con.scale);
					glUniform1f(parConProgLocs[9], osz);
					glUniform1i(parConProgLocs[10], 3);
					glActiveTexture(GL_TEXTURE3);
					if (!useGradCol) {
						glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
						glUniform1i(parConProgLocs[11], 4);
						glActiveTexture(GL_TEXTURE4);
						glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);
					}
					else {
						glBindTexture(GL_TEXTURE_BUFFER, Particles::particles_Params[gradColParam]->texBuf);
						glUniform4fv(parConProgLocs[12], 3, &gradCols[0][0]);
					}
					glUniform1i(parConProgLocs[13], useGradCol);
					glUniform1i(parConProgLocs[14], useConGradCol);
					glUniform4f(parConProgLocs[15], conCol.r, conCol.g, conCol.b, useConCol? 1 : 0);
					glUniform1f(parConProgLocs[16], spriteScl);

					glDrawArrays(GL_TRIANGLES, p.first * 12, p.second.first * 12);
				}
			}
			uint id2 = 4;
			for (auto& c2 : Particles::particles_Conn2) {
				id2++;
				if (!c2.cnt || !c2.visible) continue;
				if (!c2.drawMode) {
					glUseProgram(parConLineProg);
					glUniformMatrix4fv(parConLineProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
					glUniformMatrix4fv(parConLineProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
					glUniform1i(parConLineProgLocs[2], 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
					glUniform1i(parConLineProgLocs[3], 2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_BUFFER, c2.tbuf);
					glUniform2f(parConLineProgLocs[4], c2.line_sc / Display::width, c2.line_sc / Display::height);
					glUniform1ui(parConLineProgLocs[5], id2);
					glDrawArrays(GL_TRIANGLES, 0, c2.cnt * 6);
				}
				else {
					glUseProgram(parConProg);
					glUniformMatrix4fv(parConProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
					glUniformMatrix4fv(parConProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
					glUniform3f(parConProgLocs[2], _cpos.x, _cpos.y, _cpos.z);
					glUniform3f(parConProgLocs[3], _cfwd.x, _cfwd.y, _cfwd.z);
					glUniform2f(parConProgLocs[4], _w * ql, _h * ql);
					glUniform1i(parConProgLocs[5], 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
					glUniform1i(parConProgLocs[6], 2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_BUFFER, c2.tbuf);
					glUniform1ui(parConProgLocs[7], id2);
					glUniform1f(parConProgLocs[8], c2.scale);
					glUniform1f(parConProgLocs[9], osz);
					glUniform1i(parConProgLocs[10], 3);
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
					glUniform1i(parConProgLocs[11], 4);
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);
					glUniform1i(parConProgLocs[12], 0);
					glDrawArrays(GL_TRIANGLES, 0, c2.cnt*12);

				}
			}

			glBindVertexArray(0);
			glUseProgram(0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			Protein::Draw();
			AnWeb::DrawScene();
		}
	}
}

void ParGraphics::Reblit() {
	auto cam = ChokoLait::mainCamera().get();
	if (!AnWeb::drawFull || Scene::dirty) tfboDirty = true;
	if (tfboDirty) {
		if (!!Particles::particleSz) {
			if (RayTracer::resTex) {
				UI::Quad(0, 0, static_cast<float>(Display::width), static_cast<float>(Display::height), RayTracer::resTex);
			}
			else {
				//Recolor();
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cam->blitFbos[0]);
				float zero[4] = {};
				glClearBufferfv(GL_COLOR, 0, zero);
				BlitSky();
			}
		}
	}
	//*
	//glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	Eff::Apply();

	if (tfboDirty && AnWeb::drawFull)
		tfboDirty = false;
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (Shadows::show)
		Shadows::Reblit();
	if (!!hlIds.size() || !!selIds.size())
		BlitHl();
}

void ParGraphics::BlitSky() {
	auto _p = MVP::projection();
	auto cam = ChokoLait::mainCamera().get();

	if (!usePBR) {
		glUseProgram(reflCProg);
		glUniformMatrix4fv(reflCProgLocs[0], 1, GL_FALSE, glm::value_ptr(glm::inverse(_p)));
		glUniform2f(reflCProgLocs[1], static_cast<float>(Display::width), static_cast<float>(Display::height));
		glUniform1i(reflCProgLocs[2], 0);
		glUniform1i(reflCProgLocs[3], 1);
		glUniform1i(reflCProgLocs[4], 3);
		glUniform1f(reflCProgLocs[5], reflStr);
		glUniform1f(reflCProgLocs[6], reflStrDecay);
		glUniform1f(reflCProgLocs[7], specStr);
		glUniform3f(reflCProgLocs[8], bgCol.r, bgCol.g, bgCol.b);
	}
	else {
		glUseProgram(reflProg);
		glUniformMatrix4fv(reflProgLocs[0], 1, GL_FALSE, glm::value_ptr(glm::inverse(_p)));
		glUniform2f(reflProgLocs[1], static_cast<float>(Display::width), static_cast<float>(Display::height));
		glUniform1i(reflProgLocs[2], 0);
		glUniform1i(reflProgLocs[3], 1);
		glUniform1i(reflProgLocs[5], 3);
		glUniform1i(reflProgLocs[6], 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, refl);
		glUniform1i(reflProgLocs[7], 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, reflE);
		glUniform1f(reflProgLocs[8], reflStr);
		glUniform1f(reflProgLocs[9], reflStrDecay);
		glUniform1f(reflProgLocs[10], specStr);
		if (AnWeb::drawFull)
			glUniform3f(reflProgLocs[11], 0, 0, 0);
		else
			glUniform3f(reflProgLocs[11], bgCol.r, bgCol.g, bgCol.b);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cam->texs.colTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cam->texs.normTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, cam->texs.depthTex);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(Camera::emptyVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::BlitHl() {
	glUseProgram(selHlProg);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUniform2f(selHlProgLocs[0], (float)Display::frameWidth, (float)Display::frameHeight);
	glUniform1i(selHlProgLocs[2], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->texs.idTex);
	glBindVertexArray(Camera::emptyVao);

	if (!!selIds.size()) {
		glUniform3f(selHlProgLocs[3], 0.0f, 1.0f, 0.0f);
		for (auto& i : selIds) {
			glUniform1i(selHlProgLocs[1], i);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	if (!!hlIds.size()) {
		glUniform1i(selHlProgLocs[1], hlIds[0]);
		glUniform3f(selHlProgLocs[3], 1.0f, 1.0f, 0.0f);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::DrawOverlay() {
	if (zoomFade > 0) {
		auto zf = min(zoomFade * 2, 1.0f);
		UI::Quad(Display::width * 0.5f - 150.0f, Display::height - 100.0f, 300, 20, white(zf * 0.9f, 0.15f));
		UI::Texture(Display::width * 0.5f - 150.0f, Display::height - 98.0f, 16, 16, Icons::zoomOut, white(zf));
		UI::Texture(Display::width * 0.5f + 134.0f, Display::height - 98.0f, 16, 16, Icons::zoomIn, white(zf));
		UI::Quad(Display::width * 0.5f - 130.0f, Display::height - 91.0f, 260, 2, white(zf, 0.8f));
		UI::Quad(Display::width * 0.5f - 133.0f + 260 * InverseLerp(SCL_MIN, SCL_MAX, rotScale), Display::height - 98.0f, 6, 16, white(zf));
		zoomFade -= Time::delta;
	}
}

void ParGraphics::DrawColMenu() {
	auto& exps = ParMenu::expandPos;
	float off = 20;
	UI::Label(exps - 148, off, 12, "Parameters", white());
	UI::Quad(exps - 149, off + 17, 149, 17 * 4 + 2, white(0.9f, 0.1f));
	off += 18;
	for (int a = 0; a < Particles::particles_ParamSz; a++) {
		UI::Label(exps - 147, off + a*17, 12, std::to_string(a+1), white());
		Particles::particles_ParamNms[a] = UI::EditText(exps - 130, off + a*17, 110, 16, 12, white(1, 0.4f), Particles::particles_ParamNms[a], true, white());
	}
	if (Engine::Button(exps - 130, off + Particles::particles_ParamSz*17, 110, 16, white(1, 0.4f), "+", 12, white(), true) == MOUSE_RELEASE) {
		Particles::AddParam();
	}
	off += 4 * 17 + 3;

	UI::Label(exps - 148, off, 12, "Coloring", white());
	off += 17;
	UI::alpha = (Particles::particles_ParamSz > 0)? 1 : 0.5f;
	auto ug = useGradCol;
	UI2::Toggle(exps - 148, off, 147, _("Gradient Fill"), useGradCol);
	if (Particles::particles_ParamSz == 0) useGradCol = false;
	if (ug != useGradCol) Scene::dirty = true;
	UI::alpha = 1;
	off += 17;
	if (useGradCol) {
		gradColParam = min(gradColParam, (uint)(Particles::particles_ParamSz-1));
		static Popups::DropdownItem di = Popups::DropdownItem(&gradColParam, Particles::particles_ParamNms);
		UI2::Dropdown(exps - 147, off, 146, "Param", di);
		off += 20;
		Color::DrawH2(exps - 115, off + 8, 16, 17*5 - 16, gradCols);
		static const std::string ii[] = { "0.0", "0.5", "1.0" };
		for (int a = 0; a < 3; a++) {
			UI2::Color(exps - 140, off + 34 * (2 - a), 138, ii[a], gradCols[a]);
			UI::Texture(exps - 95, off + 34 * a, 16, 16, Icons::left, white(1, 0.4f));
		}
		off += 17 * 5 + 2;
	}
	else {
		UI::Quad(exps - 148, off, 147, Display::height*0.5f - off, white(0.9f, 0.1f));
		Engine::PushStencil(exps - 148, off + 1, 147, Display::height*0.5f - off - 2);
		off++;
		UI2::sepw = 0.33f;
		for (int x = 0; x < 256; x++) {
			std::string nm = (x < Particles::defColPalleteSz) ? std::string((char*)&Particles::defColPallete[x], 2) : std::to_string(x);
			Vec3& col = Particles::colorPallete[x];
			Vec4& colt = Particles::_colorPallete[x];
			UI2::Color(exps - 145, off + 17*x, 143, nm, colt);
			if (col != Vec3(colt)) {
				col = colt;
				Particles::UpdateColorTex();
			}
		}
		UI2::sepw = 0.5f;
		off = Display::height*0.5f;
		Engine::PopStencil();
	}
	bool uc = useConCol;
	UI2::Toggle(exps - 148, off, 147, "Custom Bond Colors", useConCol);
	if (useConCol) {
		static auto cc = conCol;
		UI2::Color(exps - 147, off + 17, 146, "Bond Color", conCol);
		if (cc != conCol) {
			cc = conCol;
			Scene::dirty = true;
		}
		off += 35;
	}
	else {
		auto ucg = useConGradCol;
		UI2::Toggle(exps - 148, off + 17, 147, "Blend Colors", useConGradCol);
		if (ucg != useConGradCol) Scene::dirty = true;
		off += 35;
	}
	if (uc != useConCol) Scene::dirty = true;
}

void ParGraphics::DrawMenu() {
	float s0 = rotScale;
	float rz0 = rotZ;
	float rw0 = rotW;
	Vec3 center0 = rotCenter;

	auto& expandPos = ParMenu::expandPos;

	UI2::Dropdown(expandPos - 148, 20, 147, _("Shading"), _usePBRItems);

	float off = 37;

	UI::Label(expandPos - 148, off, 12, _("Lighting"), white());
	if (usePBR && !!_usePBRItems.target) {
		UI::Quad(expandPos - 149, off + 17, 148, 17, white(0.9f, 0.1f));
		off += 1;
		UI2::Dropdown(expandPos - 147, off + 17, 146, _("Sky"), reflItms);
		off += 17;
	}
	UI::Quad(expandPos - 149, off + 16, 148, 17 * 4 + 2, white(0.9f, 0.1f));
	reflStr = UI2::Slider(expandPos - 147, off + 17, 147, _("Strength"), 0, 5, reflStr);
	reflStrDecay = UI2::Slider(expandPos - 147, off + 17 * 2, 147, _("Falloff"), 0, 50, reflStrDecay);
	specStr = UI2::Slider(expandPos - 147, off + 17 * 3, 147, _("Specular"), 0, 1, specStr);
	UI2::Color(expandPos - 147, off + 17 * 4, 147, _("Background"), bgCol);

	off += 17 * 5 + 2;

	UI::Label(expandPos - 148, off, 12, _("Camera"), white());
	UI::Quad(expandPos - 149, off + 17, 148, 17 * 9 + 2, white(0.9f, 0.1f));
	off += 18;
	UI::Label(expandPos - 147, off, 12, _("Target"), white());
	bool htr = (rotCenterTrackId < ~0);
	auto rf = rotCenterTrackId;
	rotCenterTrackId = TryParse(UI::EditText(expandPos - 74, off, 55, 16, 12, white(1, 0.5f), htr? std::to_string(rotCenterTrackId) : "", true, white()), ~0U);
	if (htr && Engine::Button(expandPos - 91, off, 16, 16, red()) == MOUSE_RELEASE) {
		rotCenterTrackId = -1;
	}
	if (Engine::Button(expandPos - 18, off, 16, 16, white(1, 0.5f)) == MOUSE_RELEASE) {
		
	}
	rotCenter = UI2::EditVec(expandPos - 147, off + 17, 147, _("Center"), rotCenter, !htr);

	rotZs = rotZ = TryParse(UI2::EditText(expandPos - 147, off + 17 * 4, 147, _("Rotation") + " W", std::to_string(rotZ), true, Vec4(0.6f, 0.4f, 0.4f, 1)), 0.0f);
	rotWs = rotW = TryParse(UI2::EditText(expandPos - 147, off + 17 * 5, 147, _("Rotation") + " Y", std::to_string(rotW), true, Vec4(0.4f, 0.6f, 0.4f, 1)), 0.0f);

	rotScale = TryParse(UI2::EditText(expandPos - 147, off + 17 * 6, 147, _("Scale"), std::to_string(rotScale)), 0.0f);

	auto cm = ChokoLait::mainCamera.raw();
	auto ql = cm->quality;
	ql = UI2::Slider(expandPos - 147, off + 17 * 7, 147, _("Quality"), 0.25f, 1.5f, ql, std::to_string(int(ql * 100)) + "%");
	if (Engine::Button(expandPos - 91, off + 17 * 7, 16, 16, Icons::refresh) == MOUSE_RELEASE)
		ql = 1;

	if (ql != cm->quality) {
		cm->quality = ql;
		Scene::dirty = true;
	}
	bool a2 = cm->useGBuffer2;
	UI::Label(expandPos - 147, off + 17 * 8, 12, _("Use Dynamic Quality"), white());
	a2 = Engine::Toggle(expandPos - 19, off + 17 * 8, 16, Icons::checkbox, a2, white(), ORIENT_HORIZONTAL);
	if (a2 != cm->useGBuffer2) {
		cm->useGBuffer2 = a2;
		if (a2) cm->GenGBuffer2();
		Scene::dirty = true;
	}

	off += 17 * 9 + 3;

	if (a2) {
		UI::Quad(expandPos - 149, off - 2, 148, 18, white(0.9f, 0.1f));
		ql = cm->quality2;
		ql = UI2::Slider(expandPos - 147, off - 1, 147, _("Quality") + " 2", 0.25f, 1.0f, ql, std::to_string(int(ql * 100)) + "%");
		if (ql != cm->quality2) {
			cm->quality2 = ql;
			Scene::dirty = true;
		}
		off += 17;
	}

	const int ns[] = { 1, 8, 7 };
	UI::Label(expandPos - 148, off, 12, _("Clipping"), white());
	UI::Quad(expandPos - 149, off + 17, 148, 17 * ns[(int)clippingType] + 2, white(0.9f, 0.1f));
	off += 18;
	static std::string nms[] = { _("None"), _("Slice"), _("Cube"), "" };
	static Popups::DropdownItem di = Popups::DropdownItem((uint*)&clippingType, nms);
	UI2::Dropdown(expandPos - 147, off, 146, _("Mode"), di);
	if (_clippingType != clippingType) {
		_clippingType = clippingType;
		UpdateClipping();
	}
	off += 17;
	switch (clippingType) {
	case CLIPPING::NONE: break;
	case CLIPPING::PLANE:
	{
		auto c = clipPlane.center;
		auto n = clipPlane.norm;
		auto s = clipPlane.size;
		clipPlane.center = UI2::EditVec(expandPos - 147, off, 147, _("Center"), clipPlane.center, true);
		off += 17 * 3;
		clipPlane.norm = UI2::EditVec(expandPos - 147, off, 147, _("Normal"), clipPlane.norm, true);
		off += 17 * 3;
		clipPlane.size = TryParse(UI2::EditText(expandPos - 147, off, 147, _("Thickness"), std::to_string(clipPlane.size)), 0.0f);
		off += 17;
		if (c != clipPlane.center || n != clipPlane.norm || s != clipPlane.size) UpdateClipping();
		break;
	}
	case CLIPPING::CUBE:
	{
		auto c = clipCube.center;
		auto s = clipCube.size;
		clipCube.center = UI2::EditVec(expandPos - 147, off, 147, _("Center"), clipCube.center, true);
		off += 17 * 3;
		clipCube.size = UI2::EditVec(expandPos - 147, off, 147, _("Size"), clipCube.size, true);
		off += 17 * 3;
		if (c != clipCube.center || s != clipCube.size) UpdateClipping();
		break;
	}
	}

	off += 2;
	
	off = Eff::DrawMenu(off);

	Shadows::DrawMenu(off + 1);

	if (rf != rotCenterTrackId || s0 != rotScale || rz0 != rotZ || rw0 != rotW || center0 != rotCenter) Scene::dirty = true;
}

void ParGraphics::DrawPopupDM() {
	auto& dt = *((byte*)Popups::data);
	auto dto = dt;
	byte a = dt & 0x0f;
	byte b = dt >> 4;
	UI::Quad(Popups::pos.x - 1, Popups::pos.y - 1, 18, 18, black(0.7f));
	UI::Quad(Popups::pos.x - 1, Popups::pos.y + 15, 113, 37, black(0.7f));
	UI::Quad(Popups::pos.x, Popups::pos.y, 16, 16, white(1, 0.3f));
	UI::Texture(Popups::pos.x, Popups::pos.y, 16, 16, Icons::OfDM(dt));
	UI::Quad(Popups::pos.x, Popups::pos.y + 16, 111, 35, white(1, 0.3f));

	static const Texture as[] = { Icons::dm_none, Icons::dm_point, Icons::dm_ball, Icons::dm_vdw };
	static const Texture bs[] = { Icons::dm_none, Icons::dm_line, Icons::dm_stick };

	UI::Label(Popups::pos.x + 2, Popups::pos.y + 18, 12, _("Atoms"), white());
	for (byte i = 0; i < 4; i++) {
		if (Engine::Button(Popups::pos.x + 42 + 17 * i, Popups::pos.y + 18, 16, 16, as[i], (i == a)? yellow() : white(0.8f)) == MOUSE_RELEASE) {
			if (dt == 255) dt = 0;
			dt = (dt & 0xf0) | i;
			if (!dt) dt = 0x10;
			else if (i == 3) dt = i;
		}
	}
	UI::Label(Popups::pos.x + 2, Popups::pos.y + 35, 12, _("Bonds"), white());
	for (byte i = 0; i < 3; i++) {
		if (Engine::Button(Popups::pos.x + 42 + 17 * i, Popups::pos.y + 35, 16, 16, bs[i], ((i+1) == b)? yellow() : white(0.8f)) == MOUSE_RELEASE) {
			if (dt == 255) dt = 0; 
			dt = (dt & 0x0f) | (i << 4);
			if (a == 0 && i == 0) dt = 1;
			else if (a == 3) dt = (i << 4) + 0x02;
		}
	}

	if ((Input::mouse0State == 1) && !Engine::Button(Popups::pos.x, Popups::pos.y + 16, 111, 60)) {
		Popups::type = POPUP_TYPE::NONE;
	}
	//if (dto != dt) ParGraphics::UpdateDrawLists();
}

#define SVS(nm, vl) n->addchild(#nm, vl)
#define SV(nm, vl) SVS(nm, std::to_string(vl))
void ParGraphics::Serialize(XmlNode* nd) {
	nd->name = "PGraphics";
	auto gp = nd->addchild("Graphics");
	auto n = gp->addchild("Lighting");
	SV(shading, (int)usePBR); SV(sky, reflId); SV(skystr, reflStr);
	SV(skyfall, reflStrDecay), SV(specstr, specStr);
	n->children.push_back(Xml::FromVec("bgcol", bgCol));

	n = gp->addchild("Camera");
	SV(target, rotCenterTrackId);
	n->children.push_back(Xml::FromVec("center", rotCenter));
	SV(rotw, rotW); SV(rotz, rotZ); SV(scale, rotScale);
	auto cm = ChokoLait::mainCamera.raw();
	SV(quality, cm->quality);
	SVS(usequality2, cm->useGBuffer2? "1" : "0");
	SV(quality2, cm->quality2);
	SerializeCol(nd->addchild("Colors"));
	Eff::Serialize(nd->addchild("Effects"));
	Shadows::Serialize(nd->addchild("Shadows"));
}
void ParGraphics::SerializeCol(XmlNode* n) {
	auto pm = n->addchild("Params");
	for (int a = 0; a < Particles::particles_ParamSz; a++) {
		pm->addchild("item")
			->addchild("name", Particles::particles_ParamNms[a]);
	}
	SV(usegrad, (int)useGradCol);
	/*
	auto gr = pm->addchild("grad");
	for (int a = 0; a < 3; a++) {
		gr->children.push_back(Xml::FromVec(std::to_string(a), gradCols[a]));
	}
	*/
	SV(useconcol, (int)useConCol); SV(usecongradcol, (int)useConGradCol);
	n->children.push_back(Xml::FromVec("concol", conCol));
}
#undef SVS
#undef SV

void ParGraphics::Deserialize(XmlNode* nd) {
#define SW(nm) if (n3.name == #nm) for (auto& n4 : n3.children)
#define GT(nm, vl) if (n4.name == #nm) vl = TryParse(n4.value, vl)
#define GTV(nm, vl) if (n4.name == #nm) Xml::ToVec(&n4, vl)
	for (auto& n : nd->children) {
		if (n.name == "PGraphics") {
			for (auto& n2 : n.children) {
				if (n2.name == "Graphics") {
					for (auto& n3 : n2.children) {
						SW(Lighting) {
							GT(shading, usePBR);
							else GT(sky, reflId);
							else GT(skystr, reflStr);
							else GT(skyfall, reflStrDecay);
							else GT(specstr, specStr);
							else GTV(bgcol, bgCol);
						}
						else SW(Camera) {
							auto cm = ChokoLait::mainCamera.raw();
							GT(target, rotCenterTrackId);
							else GTV(center, rotCenter);
							else GT(rotw, rotW);
							else GT(rotz, rotZ);
							else GT(scale, rotScale);
							else GT(quality, cm->quality);
							else if (n4.name == "usequality2") cm->useGBuffer2 = (n4.value == "1");
							else GT(quality2, cm->quality2);
						}
					}
				}
				else if (n2.name == "Colors") DeserializeCol(&n2);
				else if (n2.name == "Effects") Eff::Deserialize(&n2);
				else if (n2.name == "Shadows") Shadows::Deserialize(&n2);
			}
			
			rotWs = rotW;
			rotZs = rotZ;
			return;
		}
	}
#undef SW
#undef GT
#undef GTV
}
void ParGraphics::DeserializeCol(XmlNode* nd) {
#define GTS(nm, vl) if (n.name == #nm) vl = n.value
#define GT(nm, vl) if (n.name == #nm) vl = TryParse(n.value, vl)
#define GTB(nm, vl) if (n.name == #nm) vl = (n.value == "1")
#define GTV(nm, vl) if (n.name == #nm) Xml::ToVec(&n, vl)
	for (auto& n : nd->children) {
		if (n.name == "Params") {
			auto& ps = Particles::particles_ParamSz;
			for (auto& n2 : n.children) {
				if (n2.name == "item") {
					for (auto& n : n2.children) {
						Particles::AddParam();
						GTS(name, Particles::particles_ParamNms[ps-1]);
					}
				}
			}
		}
		else GTB(usegrad, useGradCol);
		else GTB(useconcol, useConCol);
		else GTB(usecongradcol, useConGradCol);
		else GTV(concol, conCol);
	}
#undef GTS
#undef GT
#undef GTB
#undef GTV
}