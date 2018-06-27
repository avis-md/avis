#include "pargraphics.h"
#include "md/ParMenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "vis/system.h"
#include "ui/icons.h"
#include "ui/popups.h"
#include "utils/effects.h"
#include "web/anweb.h"
#include "mdchan.h"

Texture* ParGraphics::refl = nullptr, *ParGraphics::bg = nullptr, *ParGraphics::logo = nullptr;
float ParGraphics::reflStr = 1, ParGraphics::reflStrDecay = 2, ParGraphics::rimOff = 0.5f, ParGraphics::rimStr = 1;

Light* ParGraphics::light;

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
	
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, Display::width, Display::height, 0, 0, Display::width, Display::height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void ParGraphics::Eff::DrawMenu(float off) {
	auto& expandPos = ParMenu::expandPos;
	
	UI::Label(expandPos - 148, off, 12, "Effects", white());

	off += 17;
	Engine::DrawQuad(expandPos - 148, off, 146, 17 * 5, white(0.9f, 0.1f));
	UI::Label(expandPos - 146, off, 12, "Ambient Occlusion", white());
	useSSAO = Engine::Toggle(expandPos - 20, off, 16, Icons::checkbox, useSSAO, white(), ORIENT_HORIZONTAL);
	UI::Label(expandPos - 145, off + 17, 12, "Samples", white());
	ssaoSamples = TryParse(UI::EditText(expandPos - 80, off + 17, 76, 16, 12, white(1, 0.5f), std::to_string(ssaoSamples), true, white()), 0);
	ssaoSamples = Clamp(ssaoSamples, 10, 100);
	UI::Label(expandPos - 145, off + 34, 12, "Radius", white());
	ssaoRad = Engine::DrawSliderFill(expandPos - 80, off + 34, 76, 16, 0.001f, 0.05f, ssaoRad, white(1, 0.5f), white());
	UI::Label(expandPos - 145, off + 51, 12, "Strength", white());
	ssaoStr = Engine::DrawSliderFill(expandPos - 80, off + 51, 76, 16, 0, 3, ssaoStr, white(1, 0.5f), white());
	UI::Label(expandPos - 145, off + 68, 12, "Blur", white());
	ssaoBlur = Engine::DrawSliderFill(expandPos - 80, off + 68, 76, 16, 0, 40, ssaoBlur, white(1, 0.5f), white());
	
}


void ParGraphics::Init() {
	refl = new Texture(IO::path + "/refl.png", true, TEX_FILTER_BILINEAR, 1, GL_REPEAT, GL_MIRRORED_REPEAT);
	bg = new Texture(IO::path + "/res/bg.jpg", false, TEX_FILTER_BILINEAR, 1, TEX_WRAP_CLAMP);
	logo = new Texture(IO::path + "/res/logo.png", false, TEX_FILTER_BILINEAR, 1, TEX_WRAP_CLAMP);
	//reflProg = (new Shader(DefaultResources::GetStr("lightPassVert.txt"), IO::GetText(IO::path + "/reflFrag.txt")))->pointer;
	reflProg = Shader::FromVF(IO::GetText(IO::path + "/minVert.txt"), IO::GetText(IO::path + "/reflFrag.txt"));
	reflProgLocs[0] = glGetUniformLocation(reflProg, "_IP");
	reflProgLocs[1] = glGetUniformLocation(reflProg, "screenSize");
	reflProgLocs[2] = glGetUniformLocation(reflProg, "inColor");
	reflProgLocs[3] = glGetUniformLocation(reflProg, "inNormal");
	reflProgLocs[4] = glGetUniformLocation(reflProg, "inEmit");
	reflProgLocs[5] = glGetUniformLocation(reflProg, "inDepth");
	reflProgLocs[6] = glGetUniformLocation(reflProg, "inSky");
	reflProgLocs[7] = glGetUniformLocation(reflProg, "skyStrength");
	reflProgLocs[8] = glGetUniformLocation(reflProg, "skyStrDecay");
	reflProgLocs[9] = glGetUniformLocation(reflProg, "rimOffset");
	reflProgLocs[10] = glGetUniformLocation(reflProg, "rimStrength");
	
	parProg = Shader::FromVF(IO::GetText(IO::path + "/parV.txt"), IO::GetText(IO::path + "/parF.txt"));
	parProgLocs[0] = glGetUniformLocation(parProg, "_MV");
	parProgLocs[1] = glGetUniformLocation(parProg, "_P");
	parProgLocs[2] = glGetUniformLocation(parProg, "camPos");
	parProgLocs[3] = glGetUniformLocation(parProg, "camFwd");
	parProgLocs[4] = glGetUniformLocation(parProg, "screenSize");
	parProgLocs[5] = glGetUniformLocation(parProg, "radTex");
	parProgLocs[6] = glGetUniformLocation(parProg, "radScl");

	parConProg = Shader::FromVF(IO::GetText(IO::path + "/parConV.txt"), IO::GetText(IO::path + "/parConF.txt"));
	parConProgLocs[0] = glGetUniformLocation(parConProg, "_MV");
	parConProgLocs[1] = glGetUniformLocation(parConProg, "_P");
	parConProgLocs[2] = glGetUniformLocation(parConProg, "camPos");
	parConProgLocs[3] = glGetUniformLocation(parConProg, "camFwd");
	parConProgLocs[4] = glGetUniformLocation(parConProg, "screenSize");
	parConProgLocs[5] = glGetUniformLocation(parConProg, "posTex");
	parConProgLocs[6] = glGetUniformLocation(parConProg, "connTex");

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

	hlIds.resize(1);
	ChokoLait::mainCamera->onBlit = Reblit;

	rotCenter = Vec3();//4, 4, 4);
	rotZ = 90;
	rotScale = 0;

	Eff::ssaoSamples = 20;
	Eff::ssaoRad = 0.015f;
	Eff::ssaoStr = 1;
	Eff::ssaoBlur = 6.5f;
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

void ParGraphics::SetLight(Light* l) {
	light = l;
	l->lightType = LIGHTTYPE_DIRECTIONAL;
	l->drawShadow = true;
	l->shadowOnly = true;
	l->shadowStrength = 1;
	l->maxDist = 20;
}

void ParGraphics::Update() {
	if (!Particles::particleSz)
		Scene::dirty = true;
	else if (animate && !seek) {
		Particles::IncFrame(true);
		Scene::dirty = true;
	}
	if (!UI::editingText) {
		float s0 = rotScale;
		float rz0 = rotZ;
		float rw0 = rotW;
		Vec3 center0 = rotCenter;

		if (Input::mouse0) {
			if (Input::mouse0State == MOUSE_DOWN && VisSystem::InMainWin(Input::mousePos)) {
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

void ParGraphics::Rerender() {
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

		if (dragging) {
			auto imvp = glm::inverse(MVP::projection() * MVP::modelview());
			scrX = imvp * Vec4(1, 0, 0, 0);
			scrY = imvp * Vec4(0, 1, 0, 0);
		}
		auto _mv = MVP::modelview();
		auto _p = MVP::projection();
		auto _cpos = ChokoLait::mainCamera->object->transform.position();
		auto _cfwd = ChokoLait::mainCamera->object->transform.forward();

		auto ql = ChokoLait::mainCamera->quality;

		glUseProgram(parProg);
		glUniformMatrix4fv(parProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
		glUniformMatrix4fv(parProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
		glUniform3f(parProgLocs[2], _cpos.x, _cpos.y, _cpos.z);
		glUniform3f(parProgLocs[3], _cfwd.x, _cfwd.y, _cfwd.z);
		glUniform2f(parProgLocs[4], Display::width * ql, Display::height * ql);
		glUniform1i(parProgLocs[5], 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, Particles::radTexBuffer);

		glBindVertexArray(Particles::posVao);
		for (auto& p : drawLists) {
			//glPolygonMode(GL_FRONT_AND_BACK, (p.second.second == 0x0f) ? GL_POINT : GL_FILL);
			if (p.second.second == 1) glUniform1f(parProgLocs[6], -1);
			else if (p.second.second == 0x0f) glUniform1f(parProgLocs[6], 0);
			else if (p.second.second == 2) glUniform1f(parProgLocs[6], 0.2f);
			else glUniform1f(parProgLocs[6], 1);
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
				glUniform2f(parConProgLocs[4], Display::width * ql, Display::height * ql);
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
				glUniform2f(parConProgLocs[4], Display::width * ql, Display::height * ql);
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
		float zero[] = { 0,0,0,0 };
		glClearBufferfv(GL_COLOR, 0, zero);
		if (!!Particles::particleSz) {
			Recolor();
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cam->d_tfbo[0]);
			BlitSky();
		}
		else {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			UI::Texture(0, 0, (float)Display::width, (float)Display::height, bg, DRAWTEX_CROP);
			MdChan::Draw(Vec2(Display::width * 0.5f, Display::height * 0.3f));
			UI::Texture(Display::width * 0.5f - Display::height * 0.2f, Display::height * 0.4f, Display::height * 0.4f, Display::height * 0.2f, logo);
			if (ParLoader::busy) {
				Engine::DrawQuad(Display::width * 0.5f - 50, Display::height * 0.6f, 100, 6, white(0.8f, 0.2f));
				Engine::DrawQuad(Display::width * 0.5f - 50, Display::height * 0.6f, 100 * ParLoader::loadProgress, 6, Vec4(0.9f, 0.7f, 0.2f, 1));
				UI::Label(Display::width * 0.5f - 48, Display::height * 0.6f + 10, 12, ParLoader::loadName);
			}
			else {
				UI::font->Align(ALIGN_TOPCENTER);
				UI::Label(Display::width * 0.5f, Display::height * 0.6f, 12, "Press F1 for Help", white());
				UI::Label(Display::width * 0.5f, Display::height * 0.6f + 14, 12, "Build: " __DATE__, white());
				UI::font->Align(ALIGN_TOPLEFT);
			}
		}
	}
	//*
	glViewport(0, 0, Display::actualWidth, Display::actualHeight);
	//glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	Eff::Apply();

	if (tfboDirty && AnWeb::drawFull)
		tfboDirty = false;
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (!!hlIds.size() || !!selIds.size())
		BlitHl();
}

void ParGraphics::BlitSky() {
	auto _p = MVP::projection();
	auto cam = ChokoLait::mainCamera().get();

	glUseProgram(reflProg);
	glUniformMatrix4fv(reflProgLocs[0], 1, GL_FALSE, glm::value_ptr(glm::inverse(_p)));
	glUniform2f(reflProgLocs[1], (float)Display::actualWidth, (float)Display::actualHeight);
	glUniform1i(reflProgLocs[2], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cam->d_colTex);
	glUniform1i(reflProgLocs[3], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cam->d_texs[1]);
	glUniform1i(reflProgLocs[4], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, cam->d_texs[3]);
	glUniform1i(reflProgLocs[5], 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, cam->d_depthTex);
	glUniform1i(reflProgLocs[6], 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, refl->pointer);
	glUniform1f(reflProgLocs[7], reflStr);
	glUniform1f(reflProgLocs[8], reflStrDecay);
	glUniform1f(reflProgLocs[9], rimOff);
	glUniform1f(reflProgLocs[10], rimStr);

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
	UI::Label(expandPos - 148, 3, 12, "Ambient", white());
	Engine::DrawQuad(expandPos - 149, 18, 148, 36, white(0.9f, 0.1f));
	UI::Label(expandPos - 147, 20, 12, "Strength", white());
	reflStr = Engine::DrawSliderFill(expandPos - 80, 19, 78, 16, 0, 2, reflStr, white(1, 0.5f), white());
	UI::Label(expandPos - 147, 37, 12, "Falloff", white());
	reflStrDecay = Engine::DrawSliderFill(expandPos - 80, 36, 78, 16, 0, 200, reflStrDecay, white(1, 0.5f), white());
	
	UI::Label(expandPos - 148, 54, 12, "Rim Light", white());
	Engine::DrawQuad(expandPos - 149, 68, 148, 38, white(0.9f, 0.1f));
	UI::Label(expandPos - 147, 71, 12, "Offset", white());
	rimOff = Engine::DrawSliderFill(expandPos - 80, 69, 78, 16, 0, 1, rimOff, white(1, 0.5f), white());
	UI::Label(expandPos - 147, 88, 12, "Strength", white());
	rimStr = Engine::DrawSliderFill(expandPos - 80, 88, 78, 16, 0, 5, rimStr, white(1, 0.5f), white());

	UI::Label(expandPos - 148, 105, 12, "Camera", white());
	Engine::DrawQuad(expandPos - 149, 121, 165, 140, white(0.9f, 0.1f));
	UI::Label(expandPos - 147, 122, 12, "Target", white());
	bool htr = (rotCenterTrackId < ~0);
	auto rf = rotCenterTrackId;
	rotCenterTrackId = TryParse(UI::EditText(expandPos - 80, 122, 62, 16, 12, white(1, 0.5f), htr? std::to_string(rotCenterTrackId) : "", true, white()), ~0U);
	if (htr && Engine::Button(expandPos - 97, 122, 16, 16, red()) == MOUSE_RELEASE) {
		rotCenterTrackId = -1;
	}
	if (Engine::Button(expandPos - 18, 122, 16, 16, white(1, 0.5f)) == MOUSE_RELEASE) {
		
	}
	UI::Label(expandPos - 147, 139, 12, "Center X", white());
	UI::Label(expandPos - 147, 156, 12, "Center Y", white());
	UI::Label(expandPos - 147, 173, 12, "Center Z", white());
	rotCenter.x = TryParse(UI::EditText(expandPos - 80, 139, 78, 16, 12, Vec4(0.6f, 0.4f, 0.4f, 1), std::to_string(rotCenter.x), true, white(!htr ? 1 : 0.5f)), 0.0f);
	rotCenter.y = TryParse(UI::EditText(expandPos - 80, 156, 78, 16, 12, Vec4(0.4f, 0.6f, 0.4f, 1), std::to_string(rotCenter.y), true, white(!htr ? 1 : 0.5f)), 0.0f);
	rotCenter.z = TryParse(UI::EditText(expandPos - 80, 173, 78, 16, 12, Vec4(0.4f, 0.4f, 0.6f, 1), std::to_string(rotCenter.z), true, white(!htr ? 1 : 0.5f)), 0.0f);
	
	UI::Label(expandPos - 147, 191, 12, "Rotation W", white());
	UI::Label(expandPos - 147, 208, 12, "Rotation Y", white());
	rotW = TryParse(UI::EditText(expandPos - 80, 191, 78, 16, 12, Vec4(0.6f, 0.4f, 0.4f, 1), std::to_string(rotW), true, white()), 0.0f);
	rotZ = TryParse(UI::EditText(expandPos - 80, 208, 78, 16, 12, Vec4(0.4f, 0.6f, 0.4f, 1), std::to_string(rotZ), true, white()), 0.0f);
	
	UI::Label(expandPos - 147, 226, 12, "Scale", white());
	rotScale = TryParse(UI::EditText(expandPos - 80, 226, 78, 16, 12, Vec4(0.6f, 0.4f, 0.4f, 1), std::to_string(rotScale), true, white()), 0.0f);
	rotScale = Clamp(rotScale, -6.0f, 2.0f);

	UI::Label(expandPos - 147, 243, 12, "Quality", white());
	auto cm = ChokoLait::mainCamera.raw();
	auto ql = cm->quality;
	if (Engine::Button(expandPos - 97, 243, 16, 16, Icons::refresh) == MOUSE_RELEASE)
		ql = 1;
	ql = Engine::DrawSliderFill(expandPos - 80, 243, 78, 16, 0.25f, 1.5f, ql, white(1, 0.5f), white());
	UI::Label(expandPos - 78, 243, 12, std::to_string(int(ql * 100)) + "%", black(0.6f));
	if (ql != cm->quality) {
		cm->quality = ql;
		Scene::dirty = true;
	}
	bool a2 = cm->useGBuffer2;
	UI::Label(expandPos - 147, 260, 12, "Use Dynamic Quality", white());
	a2 = Engine::Toggle(expandPos - 19, 260, 16, Icons::checkbox, a2, white(), ORIENT_HORIZONTAL);
	if (a2 != cm->useGBuffer2) {
		cm->useGBuffer2 = a2;
		if (a2) cm->GenGBuffer2();
		Scene::dirty = true;
	}
	if (a2) {
		Engine::DrawQuad(expandPos - 149, 277, 148, 18, white(0.9f, 0.1f));
		UI::Label(expandPos - 147, 277, 12, "Quality 2", white());
		ql = cm->quality2;
		ql = Engine::DrawSliderFill(expandPos - 80, 277, 78, 16, 0.25f, 1, ql, white(1, 0.5f), white());
		UI::Label(expandPos - 78, 277, 12, std::to_string(int(ql * 100)) + "%", black(0.6f));
		if (ql != cm->quality2) {
			cm->quality2 = ql;
			Scene::dirty = true;
		}
	}

	Eff::DrawMenu(a2? 294.0f : 277.0f);

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

	if (Input::mouse0 && !Engine::Button(Popups::pos.x, Popups::pos.y + 16, 111, 60)) {
		Popups::type = POPUP_TYPE::NONE;
	}
	if (dto != dt) ParGraphics::UpdateDrawLists();
}