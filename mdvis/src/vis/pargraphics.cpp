#include "pargraphics.h"
#include "md/ParMenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "vis/system.h"
#include "ui/icons.h"
#include "ui/popups.h"
#include "ui/ui_ext.h"
#include "utils/effects.h"
#include "web/anweb.h"
#include "mdchan.h"
#include "shadows.h"
#include "hdr.h"
#include "ocl/raytracer.h"

Texture* ParGraphics::bg = nullptr, *ParGraphics::logo = nullptr;
GLuint ParGraphics::refl, ParGraphics::reflE;
float ParGraphics::reflStr = 2, ParGraphics::reflStrDecay = 2, ParGraphics::specStr = 0.2f;
Vec4 ParGraphics::bgCol = Vec4(1, 1, 1, 1);

bool ParGraphics::useGradCol = false;

GLuint ParGraphics::reflProg, ParGraphics::parProg, ParGraphics::parConProg, ParGraphics::parConLineProg;
GLint ParGraphics::reflProgLocs[] = {}, ParGraphics::parProgLocs[] = {}, ParGraphics::parConProgLocs[] = {}, ParGraphics::parConLineProgLocs[] = {};

GLuint ParGraphics::selHlProg, ParGraphics::colProg;
GLint ParGraphics::selHlProgLocs[] = {}, ParGraphics::colProgLocs[] = {};

std::vector<uint> ParGraphics::hlIds, ParGraphics::selIds;
std::vector<std::pair<uint, std::pair<uint, byte>>> ParGraphics::drawLists, ParGraphics::drawListsB;

Vec3 ParGraphics::rotCenter = Vec3();
uint ParGraphics::rotCenterTrackId = -1;
float ParGraphics::rotW = 0, ParGraphics::rotZ = 0;
float ParGraphics::rotScale = 1;

float ParGraphics::zoomFade = 0;

Vec3 ParGraphics::scrX, ParGraphics::scrY;

bool ParGraphics::dragging = false;

bool ParGraphics::animate = false, ParGraphics::seek = false;
bool ParGraphics::tfboDirty = true;

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
		//cnt += Effects::Blur(cam->d_tfbo[0], cam->d_tfbo[1], cam->d_ttexs[0], cam->d_ttexs[1], _rad*20, Display::width, Display::height);
		if (useSSAO) cnt += Effects::SSAO(cam->d_tfbo[0], cam->d_tfbo[1], cam->d_tfbo[2], cam->d_ttexs[0], cam->d_ttexs[1], cam->d_ttexs[2], cam->d_texs[1], cam->d_depthTex, ssaoStr, ssaoSamples, ssaoRad, ssaoBlur, Display::width, Display::height);

		if ((cnt % 2) == 1) {
			std::swap(cam->d_tfbo[0], cam->d_tfbo[1]);
			std::swap(cam->d_ttexs[0], cam->d_ttexs[1]);
			cnt = 0;
		}
		if (AnWeb::drawFull) cnt += Effects::Blur(cam->d_tfbo[0], cam->d_tfbo[1], cam->d_ttexs[0], cam->d_ttexs[1], 1.0f, Display::width, Display::height);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, cam->d_tfbo[cnt % 2]);
	
	glViewport(0, 0, Display::actualWidth, Display::actualHeight);

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, Display::width, Display::height, 0, 0, Display::actualWidth, Display::actualHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

float ParGraphics::Eff::DrawMenu(float off) {
	auto& expandPos = ParMenu::expandPos;
	
	UI::Label(expandPos - 148, off, 12, "Effects", white());

	off += 17;
	Engine::DrawQuad(expandPos - 148, off - 1, 147, 17 * 5 + 2, white(0.9f, 0.1f));
	UI::Label(expandPos - 146, off, 12, "Ambient Occlusion", white());
	useSSAO = Engine::Toggle(expandPos - 19, off, 16, Icons::checkbox, useSSAO, white(), ORIENT_HORIZONTAL);
	ssaoSamples = (int)UI2::Slider(expandPos - 147, off + 17, 147, "Samples", 5, 100, ssaoSamples, std::to_string(ssaoSamples));
	ssaoSamples = Clamp(ssaoSamples, 10, 100);
	//UI::Label(expandPos - 145, off + 34, 12, "Radius", white());
	//ssaoRad = Engine::DrawSliderFill(expandPos - 80, off + 34, 76, 16, 0.001f, 0.05f, ssaoRad, white(1, 0.5f), white());
	ssaoRad = UI2::Slider(expandPos - 147, off + 17 * 2, 147, "Radius", 0.001f, 0.05f, ssaoRad);
	//UI::Label(expandPos - 145, off + 51, 12, "Strength", white());
	//ssaoStr = Engine::DrawSliderFill(expandPos - 80, off + 51, 76, 16, 0, 3, ssaoStr, white(1, 0.5f), white());
	ssaoStr = UI2::Slider(expandPos - 147, off + 17 * 3, 147, "Strength", 0, 3, ssaoStr);
	//UI::Label(expandPos - 145, off + 68, 12, "Blur", white());
	//ssaoBlur = Engine::DrawSliderFill(expandPos - 80, off + 68, 76, 16, 0, 40, ssaoBlur, white(1, 0.5f), white());
	ssaoBlur = UI2::Slider(expandPos - 147, off + 17 * 4, 147, "Blur", 0, 40, ssaoBlur);
	return off + 17 * 5 + 1;
}


void ParGraphics::Init() {
	uint _w, _h;
	std::vector<float> dv;
	byte* d = hdr::read_hdr((IO::path + "/res/refl_spc.hdr").c_str(), &_w, &_h);
	if (!d) {
		Debug::Error("ParGraphics", "refl_spc.hdr missing!");
		abort();
	}
	hdr::to_float(d, _w, _h, &dv);
	//byte* d = Texture::LoadPixels(IO::path + "/res/?.png", chn, _w, _h);
	glGenTextures(1, &refl);
	glBindTexture(GL_TEXTURE_2D, refl);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _w, _h, 0, GL_RGB, GL_FLOAT, &dv[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
	delete[](d);
	d = hdr::read_hdr((IO::path + "/res/refl_env.hdr").c_str(), &_w, &_h);
	if (!d) {
		Debug::Error("ParGraphics", "refl_env.hdr missing!");
		abort();
	}
	hdr::to_float(d, _w, _h, &dv);
	delete[](d);
	glGenTextures(1, &reflE);
	glBindTexture(GL_TEXTURE_2D, reflE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _w, _h, 0, GL_RGB, GL_FLOAT, &dv[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);


	bg = new Texture(IO::path + "/res/bg.jpg", false, TEX_FILTER_BILINEAR, 1, TEX_WRAP_CLAMP);
	reflProg = Shader::FromVF(IO::GetText(IO::path + "/minVert.txt"), IO::GetText(IO::path + "/reflFrag.txt"));
#define LC(nm) reflProgLocs[i++] = glGetUniformLocation(reflProg, #nm)
	uint i = 0;
	LC(_IP);
	LC(screenSize);
	LC(inColor);
	LC(inNormal);
	LC(inEmit);
	LC(inDepth);
	LC(inSky);
	LC(inSkyE);
	LC(skyStrength);
	LC(skyStrDecay);
	LC(specStr);
	LC(bgCol);
#undef LC
	
	parProg = Shader::FromVF(IO::GetText(IO::path + "/parV.txt"), IO::GetText(IO::path + "/parF.txt"));
#define LC(nm) parProgLocs[i++] = glGetUniformLocation(parProg, #nm)
	i = 0;
	LC(_MV);
	LC(_P);
	LC(camPos);
	LC(camFwd);
	LC(orthoSz);
	LC(screenSize);
	LC(radTex);
	LC(radScl);
#undef LC

	parConProg = Shader::FromVF(IO::GetText(IO::path + "/parConV.txt"), IO::GetText(IO::path + "/parConF.txt"));
#define LC(nm) parConProgLocs[i++] = glGetUniformLocation(parConProg, #nm)
	i = 0;
	LC(_MV);
	LC(_P);
	LC(camPos);
	LC(camFwd);
	LC(screenSize);
	LC(posTex);
	LC(connTex);
#undef LC

	parConLineProg = Shader::FromVF(IO::GetText(IO::path + "/parConV_line.txt"), IO::GetText(IO::path + "/parConF_line.txt"));
	parConLineProgLocs[0] = glGetUniformLocation(parConLineProg, "_MV");
	parConLineProgLocs[1] = glGetUniformLocation(parConLineProg, "_P");
	parConLineProgLocs[2] = glGetUniformLocation(parConLineProg, "posTex");
	parConLineProgLocs[3] = glGetUniformLocation(parConLineProg, "connTex");

	selHlProg = Shader::FromVF(IO::GetText(IO::path + "/minVert.txt"), IO::GetText(IO::path + "/selectorFrag.txt"));
	selHlProgLocs[0] = glGetUniformLocation(selHlProg, "screenSize");
	selHlProgLocs[1] = glGetUniformLocation(selHlProg, "myId");
	selHlProgLocs[2] = glGetUniformLocation(selHlProg, "idTex");
	selHlProgLocs[3] = glGetUniformLocation(selHlProg, "hlCol");

	colProg = Shader::FromVF(IO::GetText(IO::path + "/minVert.txt"), IO::GetText(IO::path + "/colorerFrag.txt"));
	colProgLocs[0] = glGetUniformLocation(colProg, "idTex");
	colProgLocs[1] = glGetUniformLocation(colProg, "spTex");
	colProgLocs[2] = glGetUniformLocation(colProg, "screenSize");
	colProgLocs[3] = glGetUniformLocation(colProg, "id2col");
	colProgLocs[4] = glGetUniformLocation(colProg, "colList");
	colProgLocs[5] = glGetUniformLocation(colProg, "usegrad");


	hlIds.resize(1);
	ChokoLait::mainCamera->onBlit = Reblit;

	rotCenter = Vec3();//4, 4, 4);
	rotZ = 90;
	rotScale = 0;

	Eff::ssaoSamples = 20;
	Eff::ssaoRad = 0.015f;
	Eff::ssaoStr = 1;
	Eff::ssaoBlur = 6.5f;

	Shadows::Init();
}

void ParGraphics::UpdateDrawLists() {
	drawLists.clear();
	drawListsB.clear();
	int di = -1;
	byte dt;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& r = Particles::residueLists[i];
		if ((di == -1) && r.visible) {
			di = i;
			dt = r.drawType;
		}
		else if ((di > -1) && ((!r.visible) || (dt != r.drawType))) {
			auto& rs = Particles::residueLists[di].residues[0];
			auto& rs2 = Particles::residueLists[i - 1].residues[Particles::residueLists[i - 1].residueSz-1];
			if (!!(dt & 0x0f) && (dt != 0x11)) {
				drawLists.push_back(std::pair<uint, std::pair<uint, byte>>(rs.offset, std::pair<uint, byte>(rs2.offset - rs.offset + rs2.cnt, (dt == 0x01) ? 0x0f : (dt & 0x0f))));
			}
			auto bcnt = rs2.offset_b - rs.offset_b + rs2.cnt_b;
			if (!!bcnt && !!(dt >> 4)) drawListsB.push_back(std::pair<uint, std::pair<uint, byte>>(rs.offset_b, std::pair<uint, byte>(bcnt, dt >> 4)));
			if (!r.visible) di = -1;
			else {
				di = i;
				dt = r.drawType;
			}
		}
	}
	if (di > -1) {
		auto& rs = Particles::residueLists[di].residues[0];
		auto& rs2 = Particles::residueLists[Particles::residueListSz-1].residues[Particles::residueLists[Particles::residueListSz - 1].residueSz - 1];
		if (!!(dt & 0x0f) && (dt != 0x11)) drawLists.push_back(std::pair<uint, std::pair<uint, byte>>(rs.offset, std::pair<uint, byte>(rs2.offset - rs.offset + rs2.cnt, (dt == 0x01) ? 0x0f : (dt & 0x0f))));
		auto bcnt = rs2.offset_b - rs.offset_b + rs2.cnt_b;
		if (!!bcnt && !!(dt >> 4)) drawListsB.push_back(std::pair<uint, std::pair<uint, byte>>(rs.offset_b, std::pair<uint, byte>(bcnt, dt >> 4)));
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
		Particles::IncFrame(true);
		Scene::dirty = true;
	}
	if (!!Particles::particleSz && !UI::editingText && !UI::_layerMax) {
		float s0 = rotScale;
		float rz0 = rotZ;
		float rw0 = rotW;
		Vec3 center0 = rotCenter;

		if (Input::mouse0) {
			if (Input::mouse0State == MOUSE_HOLD && !dragging && VisSystem::InMainWin(Input::mouseDownPos)) {
				dragging = true;
				ChokoLait::mainCamera->applyGBuffer2 = true;
			}
			else if (dragging) {
				if ((VisSystem::mouseMode == VIS_MOUSE_MODE::ROTATE) && !Input::KeyHold(Key_LeftShift)) {
					rotW -= 180 * Input::mouseDelta.y / Display::height;
					rotZ += 180 * Input::mouseDelta.x / Display::width;
				}
				else if ((VisSystem::mouseMode == VIS_MOUSE_MODE::PAN) || (((VisSystem::mouseMode == VIS_MOUSE_MODE::ROTATE) && Input::KeyHold(Key_LeftShift)))) {
					rotCenter -= 5.0f * scrX * (Input::mouseDelta.x / Display::width);
					rotCenter += 5.0f * scrY * (Input::mouseDelta.y / Display::height);
				}
			}
		}
		else {
			dragging = false;
			if (ChokoLait::mainCamera->applyGBuffer2) {
				ChokoLait::mainCamera->applyGBuffer2 = false;
				Scene::dirty = true;
			}
			if (Input::mouseScroll != 0 && VisSystem::InMainWin(Input::mousePos)) {
				rotScale += 0.05f * Input::mouseScroll;
				rotScale = Clamp(rotScale, -6.0f, 2.0f);
				ChokoLait::mainCamera->applyGBuffer2 = true;
				zoomFade = 2;
			}
			else {
				if (Input::KeyDown(Key_Escape)) {
					VisSystem::mouseMode = VIS_MOUSE_MODE::ROTATE;
				}
			}
		}
		if (s0 != rotScale || rz0 != rotZ || rw0 != rotW || center0 != rotCenter) Scene::dirty = true;
	}
}

void ParGraphics::Rerender(Vec3 _cpos, Vec3 _cfwd, float _w, float _h) {
	if (!!Particles::particleSz) {
		float csz = cos(-rotZ*deg2rad);
		float snz = sin(-rotZ*deg2rad);
		float csw = cos(rotW*deg2rad);
		float snw = sin(rotW*deg2rad);
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
		if (dragging) {
			auto imvp = glm::inverse(_p * _mv);
			scrX = imvp * Vec4(1, 0, 0, 0);
			scrY = imvp * Vec4(0, 1, 0, 0);
		}

		auto ql = ChokoLait::mainCamera->quality;

		float osz = -1;
		if (ChokoLait::mainCamera->ortographic) {
			Vec4 p1 = _p * Vec4(-1, 1, -1, 1);
			p1 /= p1.w;
			Vec4 p2 = _p * Vec4(1, 1, -1, 1);
			p2 /= p2.w;
			osz = glm::length(p2 - p1);
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

			glBindVertexArray(Particles::posVao);
			for (auto& p : drawLists) {
				//glPolygonMode(GL_FRONT_AND_BACK, (p.second.second == 0x0f) ? GL_POINT : GL_FILL);
				if (p.second.second == 1) glUniform1f(parProgLocs[7], -1);
				else if (p.second.second == 0x0f) glUniform1f(parProgLocs[7], 0);
				else if (p.second.second == 2) glUniform1f(parProgLocs[7], 0.2f);
				else glUniform1f(parProgLocs[7], 1);
				glDrawArrays(GL_POINTS, p.first, p.second.first);
			}

			glBindVertexArray(Camera::emptyVao);
			for (auto& p : drawListsB) {
				byte& tp = p.second.second;
				if (tp == 1) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glUseProgram(parConLineProg);
					glUniformMatrix4fv(parConLineProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
					glUniformMatrix4fv(parConLineProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
					glUniform1i(parConLineProgLocs[2], 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
					glUniform1i(parConLineProgLocs[3], 2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::connTexBuffer);
					glDrawArrays(GL_LINES, p.first * 2, p.second.first * 2);
				}
				else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
					glDrawArrays(GL_POINTS, p.first, p.second.first);
				}
			}
			for (auto& c2 : Particles::particles_Conn2) {
				if (!c2.cnt) continue;
				if (!c2.drawMode) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glUseProgram(parConLineProg);
					glUniformMatrix4fv(parConLineProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
					glUniformMatrix4fv(parConLineProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
					glUniform1i(parConLineProgLocs[2], 1);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
					glUniform1i(parConLineProgLocs[3], 2);
					glActiveTexture(GL_TEXTURE2);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_BUFFER, c2.tbuf);
					glDrawArrays(GL_LINES, 0, c2.cnt * 2);
				}
				else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
					glDrawArrays(GL_POINTS, 0, c2.cnt);
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

void ParGraphics::Recolor() {
	auto cam = ChokoLait::mainCamera.raw();

	if (Particles::palleteDirty) {
		glBindBuffer(GL_ARRAY_BUFFER, Particles::colIdBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, Particles::particleSz * sizeof(byte), Particles::particles_Col);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		Particles::palleteDirty = false;
	}

	glViewport(0, 0, int(Display::width * cam->quality), int(Display::height * cam->quality));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, cam->d_colfbo);

	glUseProgram(colProg);
	glUniform1i(colProgLocs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cam->d_idTex);
	glUniform1i(colProgLocs[1], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cam->d_texs[2]);
	glUniform2f(colProgLocs[2], Display::width * cam->quality, Display::height * cam->quality);
	glUniform1i(colProgLocs[3], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
	glUniform1i(colProgLocs[4], 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);
	glUniform1i(colProgLocs[5], useGradCol? 1 : 0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(Camera::emptyVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);
	glBindVertexArray(0);
	glViewport(0, 0, Display::actualWidth, Display::actualHeight);
}

void ParGraphics::Reblit() {
	auto cam = ChokoLait::mainCamera().get();
	if (!AnWeb::drawFull || Scene::dirty) tfboDirty = true;
	if (tfboDirty) {
		//glViewport(0, 0, Display::width, Display::height);
		float zero[] = { 0,0,0,0 };
		glClearBufferfv(GL_COLOR, 0, zero);
		if (!!Particles::particleSz) {
			if (RayTracer::resTex) {
				Engine::DrawQuad(0, 0, (float)Display::width, (float)Display::height, RayTracer::resTex);
			}
			else {
				Recolor();
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cam->d_tfbo[0]);
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

	glUseProgram(reflProg);
	glUniformMatrix4fv(reflProgLocs[0], 1, GL_FALSE, glm::value_ptr(glm::inverse(_p)));
	glUniform2f(reflProgLocs[1], (float)Display::width, (float)Display::height);
	glUniform1i(reflProgLocs[2], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cam->d_colTex);
	glUniform1i(reflProgLocs[3], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cam->d_texs[1]);
	glUniform1i(reflProgLocs[5], 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, cam->d_depthTex);
	glUniform1i(reflProgLocs[6], 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, refl);
	glUniform1i(reflProgLocs[7], 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, reflE);
	glUniform1f(reflProgLocs[8], reflStr);
	glUniform1f(reflProgLocs[9], reflStrDecay);
	glUniform1f(reflProgLocs[10], specStr);
	glUniform3f(reflProgLocs[11], bgCol.r, bgCol.g, bgCol.b);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(Camera::emptyVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::BlitHl() {
	glUseProgram(selHlProg);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glUniform2f(selHlProgLocs[0], (float)Display::actualWidth, (float)Display::actualHeight);
	glUniform1i(selHlProgLocs[2], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->d_idTex);
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

void ParGraphics::DrawMenu() {
	float s0 = rotScale;
	float rz0 = rotZ;
	float rw0 = rotW;
	Vec3 center0 = rotCenter;

	auto& expandPos = ParMenu::expandPos;
	UI::Label(expandPos - 148, 20, 12, "Lighting", white());
	Engine::DrawQuad(expandPos - 149, 37, 148, 17 * 4 + 2, white(0.9f, 0.1f));
	reflStr = UI2::Slider(expandPos - 147, 17 + 21, 147, "Strength", 0, 5, reflStr);
	reflStrDecay = UI2::Slider(expandPos - 147, 17 * 2 + 21, 147, "Falloff", 0, 50, reflStrDecay);
	specStr = UI2::Slider(expandPos - 147, 17 * 3 + 21, 147, "Specular", 0, 1, specStr);
	UI2::Color(expandPos - 147, 17 * 4 + 21, 147, "Background", bgCol);

	float off = 17 * 5 + 23;

	UI::Label(expandPos - 148, off, 12, "Camera", white());
	Engine::DrawQuad(expandPos - 149, off + 17, 148, 17 * 9 + 2, white(0.9f, 0.1f));
	off += 18;
	UI::Label(expandPos - 147, off, 12, "Target", white());
	bool htr = (rotCenterTrackId < ~0);
	auto rf = rotCenterTrackId;
	rotCenterTrackId = TryParse(UI::EditText(expandPos - 74, off, 55, 16, 12, white(1, 0.5f), htr? std::to_string(rotCenterTrackId) : "", true, white()), ~0U);
	if (htr && Engine::Button(expandPos - 91, off, 16, 16, red()) == MOUSE_RELEASE) {
		rotCenterTrackId = -1;
	}
	if (Engine::Button(expandPos - 18, off, 16, 16, white(1, 0.5f)) == MOUSE_RELEASE) {
		
	}
	rotCenter.x = TryParse(UI2::EditText(expandPos - 147, off + 17, 147, "Center X", std::to_string(rotCenter.x), !htr, Vec4(0.6f, 0.4f, 0.4f, 1)), 0.0f);
	rotCenter.y = TryParse(UI2::EditText(expandPos - 147, off + 17 * 2, 147, "Center Y", std::to_string(rotCenter.y), !htr, Vec4(0.4f, 0.6f, 0.4f, 1)), 0.0f);
	rotCenter.z = TryParse(UI2::EditText(expandPos - 147, off + 17 * 3, 147, "Center Z", std::to_string(rotCenter.z), !htr, Vec4(0.4f, 0.4f, 0.6f, 1)), 0.0f);

	rotW = TryParse(UI2::EditText(expandPos - 147, off + 17 * 4, 147, "Rotation W", std::to_string(rotW), true, Vec4(0.6f, 0.4f, 0.4f, 1)), 0.0f);
	rotZ = TryParse(UI2::EditText(expandPos - 147, off + 17 * 5, 147, "Rotation Y", std::to_string(rotZ), true, Vec4(0.4f, 0.6f, 0.4f, 1)), 0.0f);

	rotScale = TryParse(UI2::EditText(expandPos - 147, off + 17 * 6, 147, "Scale", std::to_string(rotScale)), 0.0f);
	rotScale = Clamp(rotScale, -6.0f, 2.0f);

	//UI::Label(expandPos - 147, off + 17 * 7, 12, "Quality", white());
	auto cm = ChokoLait::mainCamera.raw();
	auto ql = cm->quality;
	//ql = Engine::DrawSliderFill(expandPos - 80, off + 17 * 7, 78, 16, 0.25f, 1.5f, ql, white(1, 0.5f), white());
	//UI::Label(expandPos - 78, off + 17 * 7, 12, std::to_string(int(ql * 100)) + "%", black(0.6f));
	ql = UI2::Slider(expandPos - 147, off + 17 * 7, 147, "Quality", 0.25f, 1.5f, ql, std::to_string(int(ql * 100)) + "%");
	if (Engine::Button(expandPos - 91, off + 17 * 7, 16, 16, Icons::refresh) == MOUSE_RELEASE)
		ql = 1;

	if (ql != cm->quality) {
		cm->quality = ql;
		Scene::dirty = true;
	}
	bool a2 = cm->useGBuffer2;
	UI::Label(expandPos - 147, off + 17 * 8, 12, "Use Dynamic Quality", white());
	a2 = Engine::Toggle(expandPos - 19, off + 17 * 8, 16, Icons::checkbox, a2, white(), ORIENT_HORIZONTAL);
	if (a2 != cm->useGBuffer2) {
		cm->useGBuffer2 = a2;
		if (a2) cm->GenGBuffer2();
		Scene::dirty = true;
	}

	off += 17 * 9 + 3;

	if (a2) {
		Engine::DrawQuad(expandPos - 149, off - 2, 148, 18, white(0.9f, 0.1f));
		UI::Label(expandPos - 147, off - 1, 12, "Quality 2", white());
		ql = cm->quality2;
		ql = Engine::DrawSliderFill(expandPos - 80, off - 1, 78, 16, 0.25f, 1, ql, white(1, 0.5f), white());
		UI::Label(expandPos - 78, off - 1, 12, std::to_string(int(ql * 100)) + "%", black(0.6f));
		if (ql != cm->quality2) {
			cm->quality2 = ql;
			Scene::dirty = true;
		}
		off += 17;
	}

	off = Eff::DrawMenu(off);

	Shadows::DrawMenu(off + 1);

	rotW = Clamp<float>(rotW, -90, 90);
	rotZ = Repeat<float>(rotZ, 0, 360);

	if (rf != rotCenterTrackId || s0 != rotScale || rz0 != rotZ || rw0 != rotW || center0 != rotCenter) Scene::dirty = true;
}

void ParGraphics::DrawPopupDM() {
	auto& dt = *((byte*)Popups::data);
	auto dto = dt;
	byte a = dt & 0x0f;
	byte b = dt >> 4;
	Engine::DrawQuad(Popups::pos.x - 1, Popups::pos.y - 1, 18, 18, black(0.7f));
	Engine::DrawQuad(Popups::pos.x - 1, Popups::pos.y + 15, 113, 37, black(0.7f));
	Engine::DrawQuad(Popups::pos.x, Popups::pos.y, 16, 16, white(1, 0.3f));
	UI::Texture(Popups::pos.x, Popups::pos.y, 16, 16, Icons::OfDM(dt));
	Engine::DrawQuad(Popups::pos.x, Popups::pos.y + 16, 111, 35, white(1, 0.3f));

	UI::Label(Popups::pos.x + 2, Popups::pos.y + 18, 12, "Atoms", white());
	for (byte i = 0; i < 4; i++) {
		if (Engine::Button(Popups::pos.x + 42 + 17 * i, Popups::pos.y + 18, 16, 16, (&Icons::dm_none)[i], (i == a)? yellow() : white(0.8f)) == MOUSE_RELEASE) {
			if (dt == 255) dt = 0;
			dt = (dt & 0xf0) | i;
			if (!dt) dt = 0x10;
			else if (i == 3) dt = i;
		}
	}
	UI::Label(Popups::pos.x + 2, Popups::pos.y + 35, 12, "Bonds", white());
	if (Engine::Button(Popups::pos.x + 42, Popups::pos.y + 35, 16, 16, Icons::dm_none, (!b) ? yellow() : white(0.8f)) == MOUSE_RELEASE) {
		if (dt == 255) dt = 0;
		dt &= 0x0f;
		if (a == 0) dt = 1;
	}
	for (byte i = 0; i < 2; i++) {
		if (Engine::Button(Popups::pos.x + 59 + 17 * i, Popups::pos.y + 35, 16, 16, (&Icons::dm_line)[i], ((i+1) == b)? yellow() : white(0.8f)) == MOUSE_RELEASE) {
			if (dt == 255) dt = 0; 
			dt = (dt & 0x0f) | (i << 4) + 0x10;
			if (a == 3) dt = (i << 4) + 0x12;
		}
	}

	if ((Input::mouse0State == 1) && !Engine::Button(Popups::pos.x, Popups::pos.y + 16, 111, 60)) {
		Popups::type = POPUP_TYPE::NONE;
	}
	if (dto != dt) ParGraphics::UpdateDrawLists();
}