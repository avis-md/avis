#include <algorithm>
#include <random>
#include "pargraphics.h"
#include "md/parmenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "vis/system.h"
#include "vis/selection.h"
#include "vis/renderer.h"
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
#include "utils/dialog.h"
#include "utils/tinyfiledialogs.h"
#include "vis/preferences.h"

#define SCL_MIN -7.f
#define SCL_MAX 2.f

#define TUBESIZE 0.008


Texture ParGraphics::bg, ParGraphics::splash, ParGraphics::logo;
std::vector<std::string> ParGraphics::bgs;
int ParGraphics::bgi;
GLuint ParGraphics::refl, ParGraphics::reflE;
float ParGraphics::reflStr = 2, ParGraphics::reflTr = 0, ParGraphics::reflIor = 1;
float ParGraphics::reflStrDecay = 2, ParGraphics::reflStrDecayOff = 0, ParGraphics::specStr = 0.05f;
bool ParGraphics::fogUseBgCol = true;
uint ParGraphics::bgType = 0;
Vec4 ParGraphics::bgCol = Vec4(1, 1, 1, 1), ParGraphics::fogCol = Vec4(0, 0, 0, 1);
float ParGraphics::bgMul = 1;

bool ParGraphics::showbbox = true;

bool ParGraphics::showAxes = true;
float ParGraphics::axesSize = 15;
Vec4 ParGraphics::axesCols[] = {};

int ParGraphics::reflId = 0, ParGraphics::_reflId = -1;
std::vector<std::string> ParGraphics::reflNms;
Popups::DropdownItem ParGraphics::reflItms((uint*)&reflId, nullptr);

bool ParGraphics::useGradCol = false;
uint ParGraphics::gradColParam;
Vec4 ParGraphics::gradCols[] = { blue(), green(), red() };
bool ParGraphics::useConCol, ParGraphics::useConGradCol;
Vec4 ParGraphics::conCol = white();

float ParGraphics::radScl = 1;

ParGraphics::ORIENT ParGraphics::orientType = ParGraphics::ORIENT::NONE;
float ParGraphics::orientStr;
uint ParGraphics::orientParam[] = {};

Shader ParGraphics::reflProg;
Shader ParGraphics::reflCProg;
Shader ParGraphics::parProg;
Shader ParGraphics::parConProg;
Shader ParGraphics::parConLineProg;
Shader ParGraphics::selHlProg;
Shader ParGraphics::colProg;

#define CHK(vl) static auto _ ## vl = vl; if (_ ## vl != vl) { _ ## vl = vl; Scene::dirty = true; }
#define CHKT(vl) static auto _ ## vl = vl; if (_ ## vl != vl) { _ ## vl = vl; ParGraphics::tfboDirty = true; }

std::vector<uint> ParGraphics::hlIds;
std::vector<std::pair<uint, std::pair<uint, byte>>> ParGraphics::drawLists, ParGraphics::drawListsB;

uint ParGraphics::usePBR = 1;
std::string ParGraphics::_usePBRNms[] = {"", "", "\0"};
const Popups::DropdownItem ParGraphics::_usePBRItems = Popups::DropdownItem(&ParGraphics::usePBR, (std::string*)&ParGraphics::_usePBRNms[0]);

Vec3 ParGraphics::rotCenter = Vec3();
ParGraphics::ROTTRACK ParGraphics::rotCenterTrackType = ParGraphics::ROTTRACK::NONE;
uint ParGraphics::rotCenterTrackId = -1;
float ParGraphics::rotW = 0, ParGraphics::rotZ = 0;
float ParGraphics::rotWs = 0, ParGraphics::rotZs = 0;
float ParGraphics::rotScale = SCL_MIN + 1;

bool ParGraphics::autoRot = false;

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
Mat4x4 ParGraphics::lastMV, ParGraphics::lastP, ParGraphics::lastMVP;

//---------------- effects vars -------------------

bool ParGraphics::Eff::expanded;
bool ParGraphics::Eff::showSSAO, ParGraphics::Eff::showGlow, ParGraphics::Eff::showDof;
bool ParGraphics::Eff::useSSAO, ParGraphics::Eff::useGlow, ParGraphics::Eff::useDof;

float ParGraphics::Eff::glowThres = 0.7f, ParGraphics::Eff::glowRad = 1, ParGraphics::Eff::glowStr = 1;

float ParGraphics::Eff::ssaoRad, ParGraphics::Eff::ssaoStr, ParGraphics::Eff::ssaoBlur;
int ParGraphics::Eff::ssaoSamples;

float ParGraphics::Eff::dofDepth = 0, ParGraphics::Eff::dofFocal = 1, ParGraphics::Eff::dofAper = 1;
int ParGraphics::Eff::dofIter = 1;

void ParGraphics::Eff::Apply() {
	auto& cam = ChokoLait::mainCamera;
	byte cnt = 0;
	if (useGlow) {
		cnt += Effects::Glow(cam->blitFbos[0], cam->blitFbos[1], cam->blitFbos[2], cam->blitTexs[0], cam->blitTexs[1], cam->blitTexs[2],
			glowThres, glowRad, glowStr, Display::width, Display::height);
		if ((cnt % 2) == 1) {
			std::swap(cam->blitFbos[0], cam->blitFbos[1]);
			std::swap(cam->blitTexs[0], cam->blitTexs[1]);
			cnt = 0;
		}
	}

	if (useSSAO) {
		cnt += Effects::SSAO(cam->blitFbos[0], cam->blitFbos[1], cam->blitFbos[2], cam->blitTexs[0], cam->blitTexs[1], cam->blitTexs[2],
			cam->texs.normTex, cam->texs.depthTex, ssaoStr, ssaoSamples, ssaoRad, ssaoBlur, Display::width, Display::height);
		if ((cnt % 2) == 1) {
			std::swap(cam->blitFbos[0], cam->blitFbos[1]);
			std::swap(cam->blitTexs[0], cam->blitTexs[1]);
			cnt = 0;
		}
	}

	if (useDof) {
		cnt += Effects::Dof(cam->blitFbos[0], cam->blitFbos[1], cam->blitTexs[0], cam->blitTexs[1],
			cam->texs.depthTex, dofDepth / 500, dofFocal, dofAper, dofIter, Display::width, Display::height);
		if ((cnt % 2) == 1) {
			std::swap(cam->blitFbos[0], cam->blitFbos[1]);
			std::swap(cam->blitTexs[0], cam->blitTexs[1]);
			cnt = 0;
		}
	}

	if (AnWeb::drawFull) {
		cnt += Effects::Blur(cam->blitFbos[0], cam->blitFbos[1], cam->blitFbos[0], cam->blitTexs[0], cam->blitTexs[1],
		AnWeb::drawLerp, Display::width, Display::height);
		if (cnt%2 != 0) {
			std::swap(cam->blitTexs[0], cam->blitTexs[1]);
			std::swap(cam->blitFbos[0], cam->blitFbos[1]);
		}
	}
	VisSystem::BlurBack();
}

float ParGraphics::Eff::DrawMenu(float off) {
	auto& expandPos = ParMenu::expandPos;
	
	UI::Label(expandPos - 148, off, 12, _("Effects"), white());

	off += 17;
	UI2::Toggle(expandPos - 147, off, 145, _("Glow"), useGlow);
	if (useGlow) {
		glowThres = UI2::Slider(expandPos - 147, off + 17, 146, _("Threshold"), 0, 2, glowThres);
		glowRad = UI2::Slider(expandPos - 147, off + 17 * 2, 146, _("Radius"), 0, 1, glowRad);
		glowStr = UI2::Slider(expandPos - 147, off + 17 * 3, 146, _("Strength"), 0, 3, glowStr);

		CHKT(glowThres) CHKT(glowRad) CHKT(glowStr)
		off += 17 * 4 + 1;
	}
	else off += 18;
	CHKT(useGlow)

	UI2::Toggle(expandPos - 147, off, 145, _("Ambient Occlusion"), useSSAO);
	if (useSSAO) {
		ssaoSamples = UI2::SliderI(expandPos - 147, off + 17, 146, _("Samples"), 5, 100, ssaoSamples);
		ssaoSamples = Clamp(ssaoSamples, 10, 100);
		ssaoRad = UI2::Slider(expandPos - 147, off + 17 * 2, 146, _("Radius"), 0.001f, 0.05f, ssaoRad);
		ssaoStr = UI2::Slider(expandPos - 147, off + 17 * 3, 146, _("Strength"), 0, 3, ssaoStr);
		ssaoBlur = UI2::Slider(expandPos - 147, off + 17 * 4, 146, _("Blur"), 0, 40, ssaoBlur);

		CHKT(ssaoSamples) CHKT(ssaoRad) CHKT(ssaoStr) CHKT(ssaoBlur)
		off += 17 * 5 + 1;
	}
	else off += 18;
	CHKT(useSSAO)

	UI2::Toggle(expandPos - 147, off, 145, _("Depth Of Field"), useDof);
	if (useDof) {
		dofDepth = UI2::Slider(expandPos - 147, off + 17, 146, _("Distance"), 0, 5, dofDepth);
		//dofFocal = UI2::Slider(expandPos - 147, off + 17 * 2, 146, _("Focal"), 0.001f, 1, dofFocal);
		dofAper = UI2::Slider(expandPos - 147, off + 17 * 2, 146, _("Aperture"), 0, 10, dofAper);
		dofIter = UI2::SliderI(expandPos - 147, off + 17 * 3, 146, _("Iterations"), 1, 10, dofIter);

		CHKT(dofDepth) CHKT(dofAper) CHKT(dofIter)
		off += 17 * 4 + 1;
	}
	else off += 18;
	CHKT(useDof)

	return off;
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
				else GT(radius, ssaoRad);
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

	auto bgfs = IO::GetFiles(IO::path + "res/bg");
	for (auto& f : bgfs) {
		if (f.size() > 4) {
			auto ff = f.substr(f.size() - 4);
			if (ff == ".png" || ff == ".jpg")
				bgs.push_back(IO::path + "res/bg/" + f);
		}
	}
	if (bgs.size() > 1) {
		std::random_device rd;
		auto rng = std::default_random_engine(rd());
		std::shuffle(bgs.begin(), bgs.end(), rng);
	}
	bg = Texture(bgs[0], false, TEX_FILTER_BILINEAR, 1, TEX_WRAP_CLAMP);
	splash = Texture(IO::path + "res/bg_splash.png", false, TEX_FILTER_BILINEAR, 1, TEX_WRAP_CLAMP);
	logo = Texture(IO::path + "res/bg_logo.png", false, TEX_FILTER_BILINEAR, 1, TEX_WRAP_CLAMP);
	GLuint mv;
	Shader::LoadShader(GL_VERTEX_SHADER, glsl::minVert, mv);
	reflProg = Shader::FromF(mv, glsl::reflFrag);
#define LC(nm) reflProg.AddUniform(#nm)
	uint i = 0;
	LC(_IP); LC(screenSize); LC(inColor); LC(inNormal);
	LC(inEmit); LC(inDepth); LC(inSky); LC(inSkyE);
	LC(skyStrength); LC(skyStrDecay); LC(skyStrDecayOff);
	LC(specStr); LC(glass), LC(ior), LC(bgType),
	LC(bgCol); LC(fogCol); LC(isOrtho);
#undef LC
	
	reflCProg = Shader::FromF(mv, glsl::reflFragC);
#define LC(nm) reflCProg.AddUniform(#nm)
	i = 0;
	LC(_IP); LC(screenSize); LC(inColor);
	LC(inNormal); LC(inDepth); LC(skyStrength);
	LC(skyStrDecay); LC(specStr); LC(bgCol);
#undef LC
	
	parProg = Shader::FromVF(IO::GetText(IO::path + "parV.glsl"), IO::GetText(IO::path + "parF.glsl"));
#define LC(nm) parProg.AddUniform(#nm)
	i = 0;
	LC(_MV); LC(_P); LC(camPos);
	LC(camFwd); LC(orthoSz); LC(screenSize);
	LC(radTex); LC(radScl); LC(id2col); LC(colList);
	LC(gradCols); LC(colUseGrad); LC(spriteScl);
	LC(oriented); LC(orienScl); LC(orienX); LC(orienY); LC(orienZ);
	LC(tubesize);
	auto bid = glGetUniformBlockIndex(parProg, "clipping");
	glUniformBlockBinding(parProg, bid, _clipBindId);
#undef LC

	parConProg = Shader::FromVF(IO::GetText(IO::path + "parConV.glsl"), IO::GetText(IO::path + "parConF.glsl"));
#define LC(nm) parConProg.AddUniform(#nm)
	i = 0;
	LC(_MV); LC(_P); LC(camPos); LC(camFwd);
	LC(screenSize); LC(posTex); LC(connTex);
	LC(radTex); LC(id2); LC(radScl); 
	LC(orthoSz); LC(id2col); LC(colList);
	LC(gradCols); LC(colUseGrad);
	LC(usegrad); LC(onecol); LC(spriteScl);
	LC(tubesize);
	bid = glGetUniformBlockIndex(parConProg, "clipping");
	glUniformBlockBinding(parConProg, bid, _clipBindId);
#undef LC

	parConLineProg = Shader::FromVF(IO::GetText(IO::path + "parConV_line.txt"), IO::GetText(IO::path + "parConF_line.txt"));
#define LC(nm) parConLineProg.AddUniform(#nm)
	i = 0;
	LC(_MV); LC(_P); LC(posTex);
	LC(connTex), LC(rad); LC(id2); LC(radScl); 
	LC(orthoSz); LC(id2col); LC(colList);
	LC(gradCols); LC(colUseGrad);
	LC(usegrad); LC(onecol); 
#undef LC

	selHlProg = Shader::FromF(mv, glsl::selectorFrag);
#define LC(nm) selHlProg.AddUniform(#nm)
	i = 0;
	LC(screenSize); LC(myId); LC(idTex); LC(hlCol);
#undef LC

	colProg = Shader::FromF(mv, glsl::colererFrag);
#define LC(nm) colProg.AddUniform(#nm)
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

	Preferences::Link("VAX", &showAxes);
	Preferences::Link("VAS", &axesSize);
	Preferences::Link("VCX", &axesCols[0]);
	Preferences::Link("VCY", &axesCols[1]);
	Preferences::Link("VCZ", &axesCols[2]);
}

void ParGraphics::InitClippingMesh() {
	Vec3 pts[14];
	for (int a = 0; a < 4; ++a) {
		pts[a] = Vec3(cosf(a*PI/2), sinf(a*PI/2), 0) * 0.05f;
		pts[a+4] = pts[a];
		pts[a+8] = pts[a]*3.f;
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
	for (uint i = 0; i < Particles::residueListSz; ++i) {
		auto& r = Particles::residueLists[i];

		if (r.drawType == 255 || !r.visibleAll) {
			for (uint j = 0; j < r.residueSz; ++j) {
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

void ParGraphics::OnLoadConfig() {
	int rx = 0;
	for (int a = 0; a < Particles::attrs.size(); ++a) {
		if (Particles::attrNms[a] == "rotx") {
			orientParam[0] = a;
			rx++;
		}
		if (Particles::attrNms[a] == "roty") {
			orientParam[1] = a;
			rx++;
		}
		if (Particles::attrNms[a] == "rotz") {
			orientParam[2] = a;
			rx++;
		}
	}
	if (rx == 3) {
		orientType = ORIENT::STRETCH;
		orientStr = 1.5f;
	}
	clipPlane.center = clipCube.center = Particles::bboxCenter;
}

void ParGraphics::FillRad(byte* rads) {
	for (auto& p : drawLists) {
		float ml = 1;
		switch (p.second.second) {
		case 1:
			for (uint a = p.first; a < p.first + p.second.first; ++a) {
				rads[a] = 255;
			}
			continue;
		case 0x0f:
			continue;
		case 2:
			ml = 0.2f; break;
		default: break;
		}
		for (uint a = p.first; a < p.first + p.second.first; ++a) {
			rads[a] = (byte)(std::min(0.1f * ml * Particles::radii[a], 0.2f) * 255 / 0.2f);
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
			auto dfi = (uint)std::floor(df);
			animOff = (df - dfi) / animTarFps;
			af += dfi;
			while (af >= Particles::anim.frameCount) af -= Particles::anim.frameCount;
			Particles::SetFrame(af);
		}
		Scene::dirty = true;
	}
	if (!!Particles::particleSz && !UI::editingText && !UI::_layerMax) {

		if (zoomFade > 0) {
			static bool scrtr = false;
			if (!Input::mouse0) scrtr = false;
			if (Rect(Display::width * 0.5f - 150.f, Display::height - 100.f, 300, 20).Inside(Input::mousePos) || scrtr) {
				zoomFade = 1;
				auto _rs = rotScale;
				rotScale = Engine::DrawSliderFill(Display::width * 0.5f - 130.f, Display::height - 98.f, 260, 16, SCL_MIN, SCL_MAX, rotScale, Vec4(), Vec4());
				Input::mouse0 = false; Input::mouse0State = MOUSE_NONE;
				if (_rs != rotScale) {
					Scene::dirty = true;
					scrtr = true;
				}
			}
		}

		rotW = Clamp<float>(rotW, -90, 90);
		rotZ = Repeat<float>(rotZ, 0, 360);
		rotScale = Clamp(rotScale, SCL_MIN, SCL_MAX);

		if (autoRot) {
			rotZ = Repeat<float>(rotZ + 30*Time::delta, 0, 360);
		}

		if (Input::mouse0) {
			if (Input::mouse0State == MOUSE_DOWN) {
				if (Input::KeyHold(KEY::LeftShift)) dragMode = 2;
				else dragMode = 0;
			}
			else if ((Input::mouse0State == MOUSE_HOLD) && !dragging && (Input::mousePos != Input::mouseDownPos) && VisSystem::InMainWin(Input::mouseDownPos)) {
				dragging = true;
				if (!ChokoLait::mainCamera->applyGBuffer2) Scene::dirty = true;
				ChokoLait::mainCamera->applyGBuffer2 = true;
			}
			else if (dragging) {
				autoRot = false;
				if ((VisSystem::mouseMode == VIS_MOUSE_MODE::ROTATE) && (dragMode != 2)) {
					rotW = rotWs;
					rotZ = rotZs;
					rotW += 180 * Input::mouseDelta.y / Display::height;
					rotZ += 180 * Input::mouseDelta.x / Display::width;
					rotW = Clamp<float>(rotW, -90, 90);
					rotZ = Repeat<float>(rotZ, 0, 360);
					rotWs = rotW;
					rotZs = rotZ;
					if (Input::KeyHold(KEY::LeftShift)) {
						const float dth = 22.5f;
						rotW = dth * std::roundf(rotW / dth);
						rotZ = dth * std::roundf(rotZ / dth);
					}
				}
				else if ((VisSystem::mouseMode == VIS_MOUSE_MODE::PAN) || (((VisSystem::mouseMode == VIS_MOUSE_MODE::ROTATE) && (dragMode == 2)))) {
					rotCenter -= 5.f * scrX * (Input::mouseDelta.x / Display::width);
					rotCenter += 5.f * scrY * (Input::mouseDelta.y / Display::height);
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
				if (Input::KeyHold(KEY::LeftShift)) {
					rotCenter -= 0.1f * scrZ * Input::mouseScroll / std::pow(2.f, rotScale);
				}
				else {
					rotScale += 0.05f * Input::mouseScroll;
					rotScale = Clamp(rotScale, SCL_MIN, SCL_MAX);
					zoomFade = 2;
				}
				ChokoLait::mainCamera->applyGBuffer2 = true;
			}
			else {
				if (Input::KeyDown(KEY::Escape)) {
					VisSystem::mouseMode = VIS_MOUSE_MODE::ROTATE;
				}
			}
		}
		CHK(rotScale) CHK(rotZ) CHK(rotW) CHK(rotCenter)

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
		if (d) {
			std::vector<float> dv(_w*_h*3);
			hdr::to_float(d, _w, _h, dv.data());
			glGenTextures(1, &refl);
			glBindTexture(GL_TEXTURE_2D, refl);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _w, _h, 0, GL_RGB, GL_FLOAT, dv.data());
			SetTexParams<>(0, GL_REPEAT, GL_MIRRORED_REPEAT);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			delete[](d);
		}
		d = hdr::read_hdr((pth + "diffuse.hdr").c_str(), &_w, &_h);
		if (d) {
			std::vector<float> dv(_w*_h*3);
			hdr::to_float(d, _w, _h, dv.data());
			delete[](d);
			glGenTextures(1, &reflE);
			glBindTexture(GL_TEXTURE_2D, reflE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _w, _h, 0, GL_RGB, GL_FLOAT, dv.data());
			SetTexParams<>(0, GL_REPEAT, GL_MIRRORED_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		ParGraphics::tfboDirty = true;
	}
}

#define TTF(f, t) *(Vec3*)&f = t
void ParGraphics::UpdateClipping() {
	auto mv = MVP::modelview();
	switch (clippingType) {
	case CLIPPING::NONE:
		for (int a = 0; a < 6; ++a) {
			clippingPlanes[a] = Vec4();
		}
		break;
	case CLIPPING::PLANE:
	{
		//auto nl = glm::length(clipPlane.norm);
		//if (nl > 0)
		//	clipPlane.norm /= nl;
		Vec4 cents[2] = {}, dirs[2] = {};
		TTF(dirs[0], glm::normalize(clipPlane.norm));
		TTF(dirs[1], -clipPlane.norm);
		TTF(cents[0], clipPlane.center + clipPlane.norm * clipPlane.size * 0.5f);
		TTF(cents[1], clipPlane.center - clipPlane.norm * clipPlane.size * 0.5f);
		cents[0].w = cents[1].w = 1;
		dirs[0] = glm::normalize(mv * dirs[0]);
		dirs[1] = glm::normalize(mv * dirs[1]);
		clippingPlanes[0] = dirs[0];
		clippingPlanes[1] = dirs[1];
		for (int a = 0; a < 2; ++a) {
			cents[a] = mv * cents[a];
			cents[a] /= cents[a].w;
			clippingPlanes[a].w = glm::dot((Vec3)cents[a], (Vec3)clippingPlanes[a]);
		}
		for (int a = 2; a < 6; ++a) {
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
		for (int a = 0; a < 3; ++a) {
			dirs[a] = glm::normalize(mv * dirs[a]);
		}
		clippingPlanes[0] = -dirs[0];
		clippingPlanes[1] = dirs[0];
		clippingPlanes[2] = -dirs[1];
		clippingPlanes[3] = dirs[1];
		clippingPlanes[4] = -dirs[2];
		clippingPlanes[5] = dirs[2];
		for (int a = 0; a < 6; ++a) {
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
	if (!Particles::particleSz) return;
	float csz = cos(-rotZ*deg2rad);
	float snz = sin(-rotZ*deg2rad);
	float csw = cos(-rotW*deg2rad);
	float snw = sin(-rotW*deg2rad);
	Mat4x4 mMatrix = Mat4x4(1, 0, 0, 0, 0, csw, snw, 0, 0, -snw, csw, 0, 0, 0, 0, 1) * Mat4x4(csz, 0, -snz, 0, 0, 1, 0, 0, snz, 0, csz, 0, 0, 0, 0, 1);
	MVP::Mul(mMatrix);
	float s = std::pow(2.f, rotScale);
	MVP::Scale(s, s, s);
	if (rotCenterTrackId < ~0) {
	}
	switch (rotCenterTrackType) {
	case ROTTRACK::ATOM:
		if (rotCenterTrackId != ~0U) {
			rotCenter = Particles::poss[rotCenterTrackId];
		}
		break;
	case ROTTRACK::BBOX:
		rotCenter = (Vec3)Particles::bboxCenter;
		break;
	default:
		break;
	}
	MVP::Translate(-rotCenter.x, -rotCenter.y, -rotCenter.z);
	auto _mv = MVP::modelview();
	auto _p = MVP::projection();

	UpdateClipping();

	if (!Shadows::isPass) {
		lastMV = _mv;
		lastP = _p;
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
		if (VisRenderer::status == VisRenderer::STATUS::BUSY) {
			osz /= VisRenderer::imgSlices;
		}
	}

	if (!RayTracer::resTex) {
		glUseProgram(parProg);
		glUniformMatrix4fv(parProg.Loc(0), 1, GL_FALSE, glm::value_ptr(_mv));
		glUniformMatrix4fv(parProg.Loc(1), 1, GL_FALSE, glm::value_ptr(_p));
		glUniform3f(parProg.Loc(2), _cpos.x, _cpos.y, _cpos.z);
		glUniform3f(parProg.Loc(3), _cfwd.x, _cfwd.y, _cfwd.z);
		glUniform1f(parProg.Loc(4), osz);
		glUniform2f(parProg.Loc(5), _w * ql, _h * ql);
		glUniform1i(parProg.Loc(6), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, Particles::radTexBuffer);
		glUniform1i(parProg.Loc(9), 3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);
		glUniform1i(parProg.Loc(8), 2);
		glActiveTexture(GL_TEXTURE2);
		if (!useGradCol) {
			glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
		}
		else {
			glBindTexture(GL_TEXTURE_BUFFER, Particles::attrs[gradColParam]->texBuf);
			glUniform4fv(parProg.Loc(10), 3, &gradCols[0][0]);
		}
		glUniform1i(parProg.Loc(11), useGradCol);
		glUniform1f(parProg.Loc(12), spriteScl);
		bool useorien = (orientType == ORIENT::STRETCH && orientStr > 0.0001f);
		glUniform1i(parProg.Loc(13), useorien ? 1 : 0);
		if (useorien) {
			glUniform1f(parProg.Loc(14), std::pow(2, orientStr));
			glUniform1i(parProg.Loc(15), 4);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_BUFFER, Particles::attrs[orientParam[0]]->texBuf);
			glUniform1i(parProg.Loc(16), 5);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_BUFFER, Particles::attrs[orientParam[1]]->texBuf);
			glUniform1i(parProg.Loc(17), 6);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_BUFFER, Particles::attrs[orientParam[2]]->texBuf);
		}
		glUniform1f(parProg.Loc(18), TUBESIZE);

		glBindVertexArray(Particles::posVao);
		for (auto& p : drawLists) {
			if (p.second.second == 1) glUniform1f(parProg.Loc(7), -1);
			else if (p.second.second == 0x0f) glUniform1f(parProg.Loc(7), 0);
			else if (p.second.second == 2) glUniform1f(parProg.Loc(7), 0.2f * radScl);
			else glUniform1f(parProg.Loc(7), radScl);
			glDrawArrays(GL_POINTS, p.first, p.second.first);
		}

		auto& con = Particles::conns;
		glBindVertexArray(Camera::emptyVao);
		for (auto& p : drawListsB) {
			byte& tp = p.second.second;
			if (tp == 1) {
				glUseProgram(parConLineProg);
				glUniformMatrix4fv(parConLineProg.Loc(0), 1, GL_FALSE, glm::value_ptr(_mv));
				glUniformMatrix4fv(parConLineProg.Loc(1), 1, GL_FALSE, glm::value_ptr(_p));
				glUniform1i(parConLineProg.Loc(2), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
				glUniform1i(parConLineProg.Loc(3), 2);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::connTexBuffer);
				glUniform2f(parConLineProg.Loc(4), con.line_sc / Display::width, con.line_sc / Display::height);
				glUniform1ui(parConLineProg.Loc(5), 1);
				glUniform1f(parConLineProg.Loc(6), con.scale);
				glUniform1f(parConLineProg.Loc(7), osz);
				glUniform1i(parConLineProg.Loc(8), 3);
				glActiveTexture(GL_TEXTURE3);
				if (!useGradCol) {
					glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
					glUniform1i(parConLineProg.Loc(9), 4);
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);
				}
				else {
					glBindTexture(GL_TEXTURE_BUFFER, Particles::attrs[gradColParam]->texBuf);
					glUniform4fv(parConLineProg.Loc(10), 3, &gradCols[0][0]);
				}
				glUniform1i(parConLineProg.Loc(11), useGradCol);
				glUniform1i(parConLineProg.Loc(12), useConGradCol);
				glUniform4f(parConLineProg.Loc(13), conCol.r, conCol.g, conCol.b, useConCol? 1.f : 0.f);
				glUniform1f(parConLineProg.Loc(14), spriteScl);
				glDrawArrays(GL_TRIANGLES, p.first * 6, p.second.first * 6);
			}
			else {
				glUseProgram(parConProg);
				glUniformMatrix4fv(parConProg.Loc(0), 1, GL_FALSE, glm::value_ptr(_mv));
				glUniformMatrix4fv(parConProg.Loc(1), 1, GL_FALSE, glm::value_ptr(_p));
				glUniform3f(parConProg.Loc(2), _cpos.x, _cpos.y, _cpos.z);
				glUniform3f(parConProg.Loc(3), _cfwd.x, _cfwd.y, _cfwd.z);
				glUniform2f(parConProg.Loc(4), _w * ql, _h * ql);
				glUniform1i(parConProg.Loc(5), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
				glUniform1i(parConProg.Loc(6), 2);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::connTexBuffer);
				glUniform1i(parConProg.Loc(7), 3);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::radTexBuffer);
				glUniform1ui(parConProg.Loc(8), 1);
				glUniform1f(parConProg.Loc(9), con.scale);
				glUniform1f(parConProg.Loc(10), osz);
				glUniform1i(parConProg.Loc(11), 4);
				glActiveTexture(GL_TEXTURE4);
				if (!useGradCol) {
					glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
					glUniform1i(parConProg.Loc(12), 5);
					glActiveTexture(GL_TEXTURE5);
					glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);
				}
				else {
					glBindTexture(GL_TEXTURE_BUFFER, Particles::attrs[gradColParam]->texBuf);
					glUniform4fv(parConProg.Loc(13), 3, &gradCols[0][0]);
				}
				glUniform1i(parConProg.Loc(14), useGradCol);
				glUniform1i(parConProg.Loc(15), useConGradCol);
				glUniform4f(parConProg.Loc(16), conCol.r, conCol.g, conCol.b, useConCol? 1.f : 0.f);
				glUniform1f(parConProg.Loc(17), spriteScl);
				glUniform1f(parConProg.Loc(18), TUBESIZE);

				glDrawArrays(GL_TRIANGLES, p.first * 12, p.second.first * 12);
			}
		}
		uint id2 = 4;
		for (auto& c2 : Particles::particles_Conn2) {
			id2++;
			if (!c2.cnt || !c2.visible) continue;
			if (!c2.drawMode) {
				glUseProgram(parConLineProg);
				glUniformMatrix4fv(parConLineProg.Loc(0), 1, GL_FALSE, glm::value_ptr(_mv));
				glUniformMatrix4fv(parConLineProg.Loc(1), 1, GL_FALSE, glm::value_ptr(_p));
				glUniform1i(parConLineProg.Loc(2), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
				glUniform1i(parConLineProg.Loc(3), 2);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_BUFFER, c2.tbuf);
				glUniform2f(parConLineProg.Loc(4), c2.line_sc / Display::width, c2.line_sc / Display::height);
				glUniform1ui(parConLineProg.Loc(5), id2);
				glDrawArrays(GL_TRIANGLES, 0, c2.cnt * 6);
			}
			else {
				glUseProgram(parConProg);
				glUniformMatrix4fv(parConProg.Loc(0), 1, GL_FALSE, glm::value_ptr(_mv));
				glUniformMatrix4fv(parConProg.Loc(1), 1, GL_FALSE, glm::value_ptr(_p));
				glUniform3f(parConProg.Loc(2), _cpos.x, _cpos.y, _cpos.z);
				glUniform3f(parConProg.Loc(3), _cfwd.x, _cfwd.y, _cfwd.z);
				glUniform2f(parConProg.Loc(4), _w * ql, _h * ql);
				glUniform1i(parConProg.Loc(5), 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
				glUniform1i(parConProg.Loc(6), 2);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_BUFFER, c2.tbuf);
				glUniform1i(parConProg.Loc(7), 3);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::radTexBuffer);
				glUniform1ui(parConProg.Loc(8), id2);
				glUniform1f(parConProg.Loc(9), c2.scale);
				glUniform1f(parConProg.Loc(10), osz);
				glUniform1i(parConProg.Loc(11), 4);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
				glUniform1i(parConProg.Loc(12), 5);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);
				glUniform1i(parConProg.Loc(14), 0);
				glUniform1i(parConProg.Loc(15), 0);
				glUniform4f(parConProg.Loc(16), c2.col.r, c2.col.g, c2.col.b, c2.usecol? 1.f : 0.f);
				glUniform1f(parConProg.Loc(17), spriteScl);
				glUniform1f(parConProg.Loc(18), TUBESIZE);
				glDrawArrays(GL_TRIANGLES, 0, c2.cnt*12);

			}
		}

		glBindVertexArray(0);
		glUseProgram(0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		Protein::Draw();
		AnWeb::DrawScene();
	}

	if (showbbox) {
#define bb(i) static_cast<float>(Particles::boundingBox[i])
		UI3::Cube(bb(0), bb(1), bb(2), bb(3), bb(4), bb(5), black());
	}
}

void ParGraphics::Reblit() {
	auto& cam = ChokoLait::mainCamera;
	//if (!AnWeb::drawFull || Scene::dirty)
	//	tfboDirty = true;
	if (tfboDirty || Scene::dirty) {
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
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				if (showAxes) {
					DrawAxes();
				}
			}
		}
		//*
		//glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		Eff::Apply();
		tfboDirty = false;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cam->target);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, cam->blitFbos[0]);
	
	glViewport(0, 0, Display::frameWidth, Display::frameHeight);

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, Display::width, Display::height, 0, 0, Display::frameWidth, Display::frameHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);


	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (Shadows::show)
		Shadows::Reblit();
	if (!!hlIds.size() || !!Selection::atoms.size())
		BlitHl();
}

void ParGraphics::BlitSky() {
	auto _p = MVP::projection();
	auto& cam = ChokoLait::mainCamera;

	if (!usePBR) {
		glUseProgram(reflCProg);
		glUniformMatrix4fv(reflCProg.Loc(0), 1, GL_FALSE, glm::value_ptr(glm::inverse(_p)));
		glUniform2f(reflCProg.Loc(1), static_cast<float>(Display::width), static_cast<float>(Display::height));
		glUniform1i(reflCProg.Loc(2), 0);
		glUniform1i(reflCProg.Loc(3), 1);
		glUniform1i(reflCProg.Loc(4), 3);
		glUniform1f(reflCProg.Loc(5), reflStr);
		glUniform1f(reflCProg.Loc(6), reflStrDecay);
		glUniform1f(reflCProg.Loc(7), specStr);
		glUniform3f(reflCProg.Loc(8), bgCol.r, bgCol.g, bgCol.b);
	}
	else {
		glUseProgram(reflProg);
		glUniformMatrix4fv(reflProg.Loc(0), 1, GL_FALSE, glm::value_ptr(glm::inverse(_p)));
		glUniform2f(reflProg.Loc(1), static_cast<float>(Display::width), static_cast<float>(Display::height));
		glUniform1i(reflProg.Loc(2), 0);
		glUniform1i(reflProg.Loc(3), 1);
		glUniform1i(reflProg.Loc(5), 3);
		glUniform1i(reflProg.Loc(6), 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, refl);
		glUniform1i(reflProg.Loc(7), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, reflE);
		glUniform1f(reflProg.Loc(8), reflStr);
		glUniform1f(reflProg.Loc(9), reflStrDecay);
		glUniform1f(reflProg.Loc(10), reflStrDecayOff);
		glUniform1f(reflProg.Loc(11), specStr);
		glUniform1f(reflProg.Loc(12), reflTr);
		glUniform1f(reflProg.Loc(13), reflIor);
		glUniform1i(reflProg.Loc(14), bgType);
		if (AnWeb::drawFull) {
			glUniform4f(reflProg.Loc(15), 0, 0, 0, 1);
			glUniform4f(reflProg.Loc(16), 0, 0, 0, 1);
		}
		else {
			if (bgType == 0 || bgCol.a < 0)
				glUniform4f(reflProg.Loc(15), bgCol.r, bgCol.g, bgCol.b, bgCol.a);
			else
				glUniform4f(reflProg.Loc(15), 0, 0, 0, bgMul);
			if (fogUseBgCol) {
				glUniform4f(reflProg.Loc(16), -1, 0, 0, bgCol.a);
			}
			else {
				glUniform4f(reflProg.Loc(16), fogCol.r, fogCol.g, fogCol.b, 1);
			}
		}
		glUniform1i(reflProg.Loc(17), cam->ortographic? 1 : 0);
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

	glUniform2f(selHlProg.Loc(0), (float)Display::frameWidth, (float)Display::frameHeight);
	glUniform1i(selHlProg.Loc(2), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->texs.idTex);
	glBindVertexArray(Camera::emptyVao);

	if (!!Selection::atoms.size()) {
		glUniform3f(selHlProg.Loc(3), 0.f, 1.f, 0.f);
		for (auto& i : Selection::atoms) {
			glUniform1i(selHlProg.Loc(1), i+1);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
	if (!!hlIds.size()) {
		glUniform1i(selHlProg.Loc(1), hlIds[0]);
		glUniform3f(selHlProg.Loc(3), 1.f, 1.f, 0.f);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::DrawAxes() {
	struct _axis {
		Vec4 pos;
		Vec4 col;
		std::string nm;
	} axis[3];
	
	axis[0] = _axis{Vec4(1, 0, 0, 0), axesCols[0], "X"};
	axis[1] = _axis{Vec4(0, 1, 0, 0), axesCols[1], "Y"};
	axis[2] = _axis{Vec4(0, 0, 1, 0), axesCols[2], "Z"};

	for (int a = 0; a < 3; a++) {
		auto& p = axis[a].pos;
		p = normalize(lastMV * p);
		p.y *= -1;
	}

	std::sort(axis, axis+3, [](const _axis& a, const _axis& b){
		return a.pos.z > b.pos.z;
	});

	const Vec2 center = Vec2(70.f + ParMenu::expandPos, Display::height - 70.f);
	const float fsz = axesSize * 12.f / 15;
	UI::font.Align(ALIGN_MIDCENTER);
	for (int a = 0; a < 3; a++) {
		const auto& ax = axis[a];
		Engine::DrawLine(center, center + Vec2(ax.pos) * axesSize, ax.col, 1);
		const auto tc = center + Vec2(ax.pos) * axesSize * 1.5f;
		UI::Label(tc.x, tc.y, fsz, ax.nm, ax.col);
	}
	UI::font.Align(ALIGN_TOPLEFT);
}

void ParGraphics::DrawOverlay() {
	if (zoomFade > 0) {
		auto zf = std::min(zoomFade * 2, 1.f);
		UI::Quad(Display::width * 0.5f - 150.f, Display::height - 100.f, 300, 20, white(zf * 0.9f, 0.15f));
		UI::Texture(Display::width * 0.5f - 150.f, Display::height - 98.f, 16, 16, Icons::zoomOut, white(zf));
		UI::Texture(Display::width * 0.5f + 134.f, Display::height - 98.f, 16, 16, Icons::zoomIn, white(zf));
		UI::Quad(Display::width * 0.5f - 130.f, Display::height - 91.f, 260, 2, white(zf, 0.8f));
		UI::Quad(Display::width * 0.5f - 133.f + 260 * InverseLerp(SCL_MIN, SCL_MAX, rotScale), Display::height - 98.f, 6, 16, white(zf));
		zoomFade -= Time::delta;
	}
}

void ParGraphics::DrawColMenu() {
	auto& exps = ParMenu::expandPos;
	float off = UI::BeginScroll(exps - 150, 19, 150, Display::height - 38);
	UI::Label(exps - 148, off, 12, "Attributes", white());
	off += 18;
	UI::Quad(exps - 149, off - 1, 149, 17 * (Particles::attrs.size() + 1) + 2, white(0.9f, 0.1f));
	for (int a = 0; a < Particles::attrs.size(); ++a) {
		UI::Label(exps - 147, off, 12, std::to_string(a+1), white());
		Particles::attrNms[a] = UI::EditText(exps - 130, off, 110, 16, 12, white(1, 0.4f), Particles::attrNms[a], true, white());
		if (!Particles::attrs[a]->readonly) {
			if (Engine::Button(exps - 19, off, 16, 16, Icons::cross, red()) == MOUSE_RELEASE) {
				Particles::RmAttr(a);
				break;
			}
		}
		off += 17;
	}
	if (Engine::Button(exps - 130, off, 110, 16, white(1, 0.4f), "+", 12, white(), true) == MOUSE_RELEASE) {
		Particles::AddAttr();
	}
	off += 18;
	if (Particles::attrs.size() > 0 && !Particles::attrs.back()->readonly) {
		if (Engine::Button(exps - 148, off, 73, 16, white(1, 0.4f), "Save", 12, white(), true) == MOUSE_RELEASE) {
			auto path = Dialog::SaveFile(".attr");
			if (!!path.size()) {
				Particles::SaveAttrs(path);
			}
		}
	}
	if (Engine::Button(exps - 74, off, 73, 16, white(1, 0.4f), "Load", 12, white(), true) == MOUSE_RELEASE) {
		auto pp = Dialog::OpenFile({});
		if (!!pp.size()) {
			auto& path = pp[0];
			Particles::LoadAttrs(path);
		}
	}

	off += 18;

	UI::Label(exps - 148, off, 12, "Colors", white());
	off += 17;
	UI::alpha = (Particles::attrs.size() > 0)? 1 : 0.5f;
	UI2::Toggle(exps - 148, off, 146, _("Gradient Fill"), useGradCol);
	if (Particles::attrs.size() == 0) useGradCol = false;
	CHK(useGradCol)
	UI::alpha = 1;
	off += 17;
	if (useGradCol) {
		gradColParam = std::min(gradColParam, (uint)(Particles::attrs.size()-1));
		static Popups::DropdownItem di = Popups::DropdownItem(&gradColParam, &Particles::attrNms[0]);
		UI2::Dropdown(exps - 147, off, 146, "Attribute", di);
		CHK(gradColParam)
		off += 20;
		Color::DrawH2(exps - 115, off + 8, 16, 17*5 - 16, gradCols);
		static const std::string ii[] = { "0.0", "0.5", "1.0" };
		static Vec4 _gradCols[3];
		for (int a = 0; a < 3; ++a) {
			UI2::Color(exps - 140, off + 34 * (2 - a), 138, ii[a], gradCols[a]);
			UI::Texture(exps - 95, off + 34 * a, 16, 16, Icons::left, white(1, 0.4f));
			if (_gradCols[a] != gradCols[a]) {
				_gradCols[a] = gradCols[a];
				Scene::dirty = true;
			}
		}
		off += 17 * 5 + 2;
	}
	else {
		auto ca = Particles::colorPallete.size();
		UI::Quad(exps - 148, off, 146, ca * 17 + 2, white(0.9f, 0.1f));
		off++;
		UI2::sepw = 0.33f;
		for (int c = 0; c < ca; ++c) {
			auto& pl = Particles::colorPallete[c];
			std::string nm = (pl.first >= *(ushort*)"A")? std::string((char*)&pl.first, 2) : std::to_string(pl.first);
			Vec3& col = pl.second;
			Vec4& colt = Particles::_colorPallete[c];
			UI2::Color(exps - 145, off, 143, nm, colt);
			if (col != Vec3(colt)) {
				col = colt;
				Particles::UpdateColorTex();
			}
			off += 17;
		}
		off += 3;

		UI::Label(exps - 147, off, 12, "Overrides", white());
		off += 18;
		UI::Quad(exps - 148, off - 1, 147, 36 * Particles::colorOverrides.size() + 19, white(0.9f, 0.1f));
		for (int a = 0; a < Particles::colorOverrides.size(); ++a) {
			auto& co = Particles::colorOverrides[a];
			UI::Quad(exps - 147, off, 145, 35, white(0.9f, 0.05f));
			co.di.target = &co.resFlags;
			UI2::Dropdown(exps - 146, off + 1, 65, co.di);
			co.type = UI::EditText(exps - 80, off + 1, 60, 16, 12, white(0.2f), co.type, true, white());
			UI2::Color(exps - 146, off + 18, 144, "Color", Particles::_colorPallete[co.colId]);
			co.Update();
			off += 36;
		}
		if (Engine::Button(exps - 130, off, 110, 16, white(1, 0.4f), "+", 12, white(), true) == MOUSE_RELEASE) {
			Particles::colorOverrides.push_back(Particles::SpecificColor());
		}
		off += 19;
	}

	UI2::Toggle(exps - 148, off, 146, "Custom Bond Colors", useConCol);
	if (useConCol) {
		UI2::Color(exps - 147, off + 17, 146, "Bond Color", conCol);
		CHK(conCol)
		off += 35;
	}
	else {
		UI2::Toggle(exps - 148, off + 17, 146, "Blend Bond Colors", useConGradCol);
		CHK(useConGradCol)
		off += 35;
	}
	CHK(useConCol)
	off += 2;

	UI::Label(exps - 148, off, 12, "Radii", white()); off += 17;
	radScl = UI2::Slider(exps - 147, off, 145, "Scale", 0.5f, 2.0f, radScl); off += 17;
	CHK(radScl);
	auto ca = Particles::colorPallete.size();
	UI::Quad(exps - 148, off, 146, ca * 17 + 2, white(0.9f, 0.1f));
	off++;
	UI2::sepw = 0.33f;
	for (auto& cp : Particles::colorPallete) {
		std::string nm = (cp.first >= *(ushort*)"A")? std::string((char*)&cp.first, 2) : std::to_string(cp.first);
		auto& rad = VisSystem::radii[cp.first];
		auto r2 = UI2::Slider(exps - 147, off, 145, nm, 0, 3, rad);
		if (r2 != rad) {
			rad = r2;
			for (int a = 0; a < Particles::particleSz; ++a) {
				if (Particles::types[a] == cp.first) {
					Particles::radii[a] = rad;
				}
			}
			Particles::UpdateRadBuf();
		}
		off += 17;
	}
	off += 3;
	UI2::sepw = 0.5f;

	UI::Label(exps - 148, off, 12, "Bounding Box", white());
	off += 18;
	UI2::Toggle(exps - 147, off, 146, "Draw", showbbox);
	CHK(showbbox)
	off += 17;
	auto _bc = Particles::bboxCenter;
	_bc = UI2::EditVec(exps - 147, off, 146, "Center", _bc, true);
	if (_bc != Particles::bboxCenter) {
		bool per = Particles::boxPeriodic;
		Particles::boxPeriodic = false;
		Particles::Rebound(_bc);
		Particles::boxPeriodic = per;
	}
	off += 17*3;
	UI2::Toggle(exps - 147, off, 146, "Periodic", Particles::boxPeriodic);
	if (Particles::boxPeriodic) {
		off += 17;
		if (Engine::Button(exps - 147, off, 71, 16, white(0.2f), "Apply", 12, white(), true) == MOUSE_RELEASE) {
			Particles::Rebound(_bc);
		}
		if (Particles::anim.frameCount > 1) {
			if (Engine::Button(exps - 75, off, 74, 16, white(0.2f), "Apply All", 12, white(), true) == MOUSE_RELEASE) {
				auto frm = Particles::anim.currentFrame;
				for (uint a = 0; a < Particles::anim.frameCount; ++a) {
					Particles::SetFrame(a);
					Particles::Rebound(_bc);
				}
				Particles::SetFrame(frm);
			}
		}
	}
	off += 19;

	static std::string ornms[] = { "None", "Stretch", "" };
	static Popups::DropdownItem ordi = Popups::DropdownItem((uint*)&orientType, ornms);
	UI2::Dropdown(exps - 148, off, 147, "Orient", ordi);
	if (Particles::attrs.size() == 0) orientType = ORIENT::NONE;
	CHK(orientType)
	off += 17;
	if (orientType != ORIENT::NONE) {
		static Popups::DropdownItem odx = Popups::DropdownItem(&orientParam[0], nullptr);
		static Popups::DropdownItem ody = Popups::DropdownItem(&orientParam[1], nullptr);
		static Popups::DropdownItem odz = Popups::DropdownItem(&orientParam[2], nullptr);
		odx.list = ody.list = odz.list = &Particles::attrNms[0];
		static uint _opx = 100, _opy = 100, _opz = 100;
		UI2::Dropdown(exps - 147, off, 146, "X", odx); off += 17;
		UI2::Dropdown(exps - 147, off, 146, "Y", ody); off += 17;
		UI2::Dropdown(exps - 147, off, 146, "Z", odz); off += 17;
		if (_opx != orientParam[0] || _opy != orientParam[1] || _opz != orientParam[2]) {
			_opx = orientParam[0]; _opy = orientParam[1]; _opz = orientParam[2];
			Scene::dirty = true;
		}
	}
	orientStr = UI2::Slider(exps - 147, off, 146, "Strength", 0, 2, orientStr); off += 17;
	CHK(orientStr)
	UI::EndScroll(off);
}

void ParGraphics::DrawMenu() {
	float s0 = rotScale;
	float rz0 = rotZ;
	float rw0 = rotW;
	Vec3 center0 = rotCenter;

	auto& expandPos = ParMenu::expandPos;

	auto off = UI::BeginScroll(expandPos - 150, 19, 150, Display::height - 38);

	UI2::Dropdown(expandPos - 148, off, 146, _("Shading"), _usePBRItems); off += 17;
	CHK(usePBR)
	UI::Label(expandPos - 148, off, 12, _("Lighting"), white());
	if (usePBR && !!_usePBRItems.target) {
		UI::Quad(expandPos - 149, off + 17, 148, 17, white(0.9f, 0.1f)); off += 1;
		UI2::Dropdown(expandPos - 147, off + 17, 146, _("Sky"), reflItms); off += 17;
	}
	off += 17;
	UI::Quad(expandPos - 149, off - 1, 148, 17 * (fogUseBgCol? 5 : 6) + 3, white(0.9f, 0.1f));
	reflStr = UI2::Slider(expandPos - 147, off, 146, _("Strength"), 0, 5, reflStr, "Illumination brightness"); off += 17;
	reflStrDecay = UI2::Slider(expandPos - 147, off, 146, _("Falloff"), 0, 500, reflStrDecay, "Exponential fade over distance"); off += 17;
	reflStrDecayOff = UI2::Slider(expandPos - 147, off, 146, _(" Offset"), 0, 0.005, reflStrDecayOff, "Exponential fade offset"); off += 17;
	UI2::Toggle(expandPos - 147, off, 146, _("Inherit Color"), fogUseBgCol); off += 17;
	if (!fogUseBgCol) { UI2::Color(expandPos - 147, off, 146, _("Color"), fogCol); off += 17; }
	specStr = UI2::Slider(expandPos - 147, off, 146, _("Specular"), 0, 1, specStr, "Reflection ratio"); off += 17;
	reflTr = UI2::Slider(expandPos - 147, off, 146, _("Transparency"), 0, 1, reflTr, "Refraction ratio"); off += 17;
	if (reflTr > 0) { reflIor = UI2::Slider(expandPos - 147, off, 146, _("IOR"), 0.5f, 5.f, reflIor, "Index of refraction"); off += 17; }
	static std::string bgTypeNms[] = { "Color", "Ambient", "Sky", "" };
	static Popups::DropdownItem bgTypeDi(&bgType, bgTypeNms);
	UI2::Dropdown(expandPos - 148, off, 146, _("Background"), bgTypeDi); off += 17;
	if (!bgType) { UI2::Color(expandPos - 147, off, 146, _(" Color"), bgCol); off += 17; }
	else { bgMul = UI2::Slider(expandPos - 147, off, 146, _(" Strength"), 0, 2, bgMul, "Background brightness"); off += 17; }

	CHKT(reflStr) CHKT(reflStrDecay) CHKT(reflStrDecayOff)
	CHKT(fogUseBgCol) CHKT(fogCol) CHKT(specStr)
	CHKT(reflTr) CHKT(reflIor) CHKT(bgCol) CHKT(bgType) CHKT(bgMul)

	off += 18;

	UI::Label(expandPos - 148, off, 12, _("Camera"), white());
	//UI::Quad(expandPos - 149, off + 17, 148, 17 * 9 + 2, white(0.9f, 0.1f));
	off += 18;
	static std::string dinms[] = { "None", "Particle", "Residue", "Residue Group", "Bounding Box", "" };
	static Popups::DropdownItem di((uint*)&rotCenterTrackType, dinms);
	UI2::sepw = 0.4f;
	UI2::Dropdown(expandPos - 147, off, 146, "Follow", di);
	UI2::sepw = 0.5f;
	CHK(rotCenterTrackType);
	off += 17;
	bool htr = (rotCenterTrackType != ROTTRACK::NONE);
	switch (rotCenterTrackType) {
	case ROTTRACK::ATOM:
		htr = (rotCenterTrackId != ~0U);
		UI::Label(expandPos - 147, off, 12, _("Target"), white());
		rotCenterTrackId = TryParse(UI::EditText(expandPos - 74, off, 55, 16, 12, white(1, 0.5f), htr? std::to_string(rotCenterTrackId) : "", true, white()), ~0U);
		if (htr && Engine::Button(expandPos - 91, off, 16, 16, red()) == MOUSE_RELEASE) {
			rotCenterTrackId = -1;
		}
		if (Engine::Button(expandPos - 18, off, 16, 16, white(1, 0.5f)) == MOUSE_RELEASE) {
			
		}
		off += 17;
		break;
	case ROTTRACK::BBOX:
		break;
	default:
		rotCenter = UI2::EditVec(expandPos - 147, off, 146, _("Center"), rotCenter, !htr);
		off += 17 * 3;
		break;
	}

	rotZs = rotZ = TryParse(UI2::EditText(expandPos - 147, off, 146, _("Rotation") + " W", std::to_string(rotZ), true, Vec4(0.6f, 0.4f, 0.4f, 1)), 0.f); off += 17;
	rotWs = rotW = TryParse(UI2::EditText(expandPos - 147, off, 146, _("Rotation") + " Y", std::to_string(rotW), true, Vec4(0.4f, 0.6f, 0.4f, 1)), 0.f); off += 17;

	rotScale = TryParse(UI2::EditText(expandPos - 147, off, 146, _("Scale"), std::to_string(rotScale)), 0.f); off += 17;

	auto& cam = ChokoLait::mainCamera;
	auto ql = cam->quality;
	ql = UI2::Slider(expandPos - 147, off, 146, _("Quality"), 0.25f, 1.5f, ql, "Resolution of the 3D view", std::to_string(int(ql * 100)) + "%");
	if (Engine::Button(expandPos - 91, off, 16, 16, Icons::refresh) == MOUSE_RELEASE)
		ql = 1;
	if (ql != cam->quality) {
		cam->quality = ql;
		Scene::dirty = true;
	}
	off += 17;
	bool a2 = cam->useGBuffer2;
	UI::Label(expandPos - 147, off, 12, _("Use Dynamic Quality"), white());
	a2 = Engine::Toggle(expandPos - 19, off, 16, Icons::checkbox, a2, white(), ORIENT_HORIZONTAL);
	if (a2 != cam->useGBuffer2) {
		cam->useGBuffer2 = a2;
		if (a2) cam->GenGBuffer2();
		//Scene::dirty = true;
	}
	off += 19;

	if (a2) {
		UI::Quad(expandPos - 149, off - 2, 148, 18, white(0.9f, 0.1f));
		ql = cam->quality2;
		ql = UI2::Slider(expandPos - 147, off - 1, 146, _("Quality") + " 2", 0.25f, 1.f, ql, "Resolution of the 3D view when moving / rotating", std::to_string(int(ql * 100)) + "%");
		if (ql != cam->quality2) {
			cam->quality2 = ql;
			//Scene::dirty = true;
		}
		off += 17;
	}

	const int ns[] = { 1, 8, 7 };
	UI::Label(expandPos - 148, off, 12, _("Clipping"), white());
	UI::Quad(expandPos - 149, off + 17, 148, 17 * ns[(int)clippingType] + 2.f, white(0.9f, 0.1f));
	off += 18;
	static std::string nms[] = { _("None"), _("Slice"), _("Cube"), "" };
	static Popups::DropdownItem di2 = Popups::DropdownItem((uint*)&clippingType, nms);
	UI2::Dropdown(expandPos - 147, off, 146, _("Mode"), di2);
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
		clipPlane.center = UI2::EditVec(expandPos - 147, off, 146, _("Center"), clipPlane.center, true);
		off += 17 * 3;
		clipPlane.norm = UI2::EditVec(expandPos - 147, off, 146, _("Normal"), clipPlane.norm, true);
		off += 17 * 3;
		clipPlane.size = TryParse(UI2::EditText(expandPos - 147, off, 146, _("Thickness"), std::to_string(clipPlane.size)), 0.f);
		off += 17;
		if (c != clipPlane.center || n != clipPlane.norm || s != clipPlane.size) UpdateClipping();
		break;
	}
	case CLIPPING::CUBE:
	{
		auto c = clipCube.center;
		auto s = clipCube.size;
		clipCube.center = UI2::EditVec(expandPos - 147, off, 146, _("Center"), clipCube.center, true);
		off += 17 * 3;
		clipCube.size = UI2::EditVec(expandPos - 147, off, 146, _("Size"), clipCube.size, true);
		off += 17 * 3;
		if (c != clipCube.center || s != clipCube.size) UpdateClipping();
		break;
	}
	}

	off += 2;
	
	off = Eff::DrawMenu(off);

	//off = Shadows::DrawMenu(off + 1);

	UI::EndScroll(off);

	if (s0 != rotScale || rz0 != rotZ || rw0 != rotW || center0 != rotCenter) Scene::dirty = true;
}

void ParGraphics::DrawPopupDM() {
	auto& dt = *((byte*)Popups::data);
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
	for (byte i = 0; i < 4; ++i) {
		if (Engine::Button(Popups::pos.x + 42 + 17 * i, Popups::pos.y + 18, 16, 16, as[i], (i == a)? yellow() : white(0.8f)) == MOUSE_RELEASE) {
			if (dt == 255) dt = 0;
			dt = (dt & 0xf0) | i;
			if (!dt) dt = 0x10;
			else if (i == 3) dt = i;
		}
	}
	UI::Label(Popups::pos.x + 2, Popups::pos.y + 35, 12, _("Bonds"), white());
	for (byte i = 0; i < 3; ++i) {
		if (Engine::Button(Popups::pos.x + 42 + 17 * i, Popups::pos.y + 35, 16, 16, bs[i], (i == b)? yellow() : white(0.8f)) == MOUSE_RELEASE) {
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
	auto& cam = ChokoLait::mainCamera;
	SV(quality, cam->quality);
	SVS(usequality2, cam->useGBuffer2? "1" : "0");
	SV(quality2, cam->quality2);
	SerializeCol(nd->addchild("Colors"));
	Eff::Serialize(nd->addchild("Effects"));
	Shadows::Serialize(nd->addchild("Shadows"));
}

void ParGraphics::SerializeCol(XmlNode* n) {
	Particles::SaveAttrs(VisSystem::currentSavePath + "_data/attrs.attr");
	SVS(attrs, "attrs.attr");
	SV(usegrad, (int)useGradCol);
	/*
	auto gr = pm->addchild("grad");
	for (int a = 0; a < 3; ++a) {
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
							auto& cam = ChokoLait::mainCamera;
							GT(target, rotCenterTrackId);
							else GTV(center, rotCenter);
							else GT(rotw, rotW);
							else GT(rotz, rotZ);
							else GT(scale, rotScale);
							else GT(quality, cam->quality);
							else if (n4.name == "usequality2") cam->useGBuffer2 = (n4.value == "1");
							else GT(quality2, cam->quality2);
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
		if (n.name == "attrs") {
			Particles::LoadAttrs(VisSystem::currentSavePath2 + n.value);
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
