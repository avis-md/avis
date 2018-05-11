#include "pargraphics.h"
#include "md/ParMenu.h"
#include "vis/system.h"

Texture* ParGraphics::refl = nullptr;
float ParGraphics::reflStr = 1, ParGraphics::reflStrDecay = 2, ParGraphics::rimOff = 0.5f, ParGraphics::rimStr = 1;

GLuint ParGraphics::reflProg, ParGraphics::parProg, ParGraphics::parConProg;
GLint ParGraphics::reflProgLocs[] = {}, ParGraphics::parProgLocs[] = {}, ParGraphics::parConProgLocs[] = {};

GLuint ParGraphics::selHlProg, ParGraphics::colProg;
GLint ParGraphics::selHlProgLocs[] = {}, ParGraphics::colProgLocs[] = {};

std::vector<uint> ParGraphics::hlIds;
std::vector<std::pair<uint, uint>> ParGraphics::drawLists, ParGraphics::drawListsB;

Vec3 ParGraphics::rotCenter = Vec3();
float ParGraphics::rotW = 0, ParGraphics::rotZ = 0;
float ParGraphics::rotScale = 1;

Vec3 ParGraphics::scrX, ParGraphics::scrY;

bool ParGraphics::dragging = false;

GLuint ParGraphics::emptyVao;

void ParGraphics::Init() {
	refl = new Texture(IO::path + "/refl.png", true, TEX_FILTER_BILINEAR, 1, GL_REPEAT, GL_MIRRORED_REPEAT);
	reflProg = (new Shader(DefaultResources::GetStr("lightPassVert.txt"), IO::GetText(IO::path + "/reflFrag.txt")))->pointer;
	reflProgLocs[0] = glGetUniformLocation(reflProg, "_IP");
	reflProgLocs[1] = glGetUniformLocation(reflProg, "screenSize");
	reflProgLocs[2] = glGetUniformLocation(reflProg, "inColor");
	reflProgLocs[3] = glGetUniformLocation(reflProg, "inNormal");
	reflProgLocs[4] = glGetUniformLocation(reflProg, "inSpec");
	reflProgLocs[5] = glGetUniformLocation(reflProg, "inDepth");
	reflProgLocs[6] = glGetUniformLocation(reflProg, "inSky");
	reflProgLocs[7] = glGetUniformLocation(reflProg, "skyStrength");
	reflProgLocs[8] = glGetUniformLocation(reflProg, "skyStrDecay");
	reflProgLocs[9] = glGetUniformLocation(reflProg, "rimOffset");
	reflProgLocs[10] = glGetUniformLocation(reflProg, "rimStrength");
	
	parProg = (new Shader(IO::GetText(IO::path + "/parV.txt"), IO::GetText(IO::path + "/parF.txt")))->pointer;
	parProgLocs[0] = glGetUniformLocation(parProg, "_MV");
	parProgLocs[1] = glGetUniformLocation(parProg, "_P");
	parProgLocs[2] = glGetUniformLocation(parProg, "camPos");
	parProgLocs[3] = glGetUniformLocation(parProg, "camFwd");
	parProgLocs[4] = glGetUniformLocation(parProg, "screenSize");

	parConProg = (new Shader(IO::GetText(IO::path + "/parConV.txt"), IO::GetText(IO::path + "/parConF.txt")))->pointer;
	parConProgLocs[0] = glGetUniformLocation(parConProg, "_MV");
	parConProgLocs[1] = glGetUniformLocation(parConProg, "_P");
	parConProgLocs[2] = glGetUniformLocation(parConProg, "camPos");
	parConProgLocs[3] = glGetUniformLocation(parConProg, "camFwd");
	parConProgLocs[4] = glGetUniformLocation(parConProg, "screenSize");
	parConProgLocs[5] = glGetUniformLocation(parConProg, "posTex");
	parConProgLocs[6] = glGetUniformLocation(parConProg, "connTex");

	selHlProg = (new Shader(DefaultResources::GetStr("lightPassVert.txt"), IO::GetText(IO::path + "/selectorFrag.txt")))->pointer;
	selHlProgLocs[0] = glGetUniformLocation(selHlProg, "screenSize");
	selHlProgLocs[1] = glGetUniformLocation(selHlProg, "myId");
	selHlProgLocs[2] = glGetUniformLocation(selHlProg, "idTex");
	selHlProgLocs[3] = glGetUniformLocation(selHlProg, "hlCol");

	colProg = (new Shader(DefaultResources::GetStr("lightPassVert.txt"), IO::GetText(IO::path + "/colorerFrag.txt")))->pointer;
	colProgLocs[0] = glGetUniformLocation(colProg, "idTex");
	colProgLocs[1] = glGetUniformLocation(colProg, "spTex");
	colProgLocs[2] = glGetUniformLocation(colProg, "screenSize");
	colProgLocs[3] = glGetUniformLocation(colProg, "id2col");
	colProgLocs[4] = glGetUniformLocation(colProg, "colList");

	glGenVertexArrays(1, &emptyVao);

	hlIds.resize(1);
	ChokoLait::mainCamera->onBlit = Reblit;
}

void ParGraphics::UpdateDrawLists() {
	drawLists.clear();
	drawListsB.clear();
	int di = -1;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& r = Particles::residueLists[i];
		if ((di == -1) && r.visible) di = i;
		else if ((di > -1) && !r.visible) {
			auto& rs = Particles::residueLists[di].residues[0];
			auto& rs2 = Particles::residueLists[i - 1].residues[Particles::residueLists[i - 1].residueSz-1];
			drawLists.push_back(std::pair<uint, uint>(rs.offset, rs2.offset - rs.offset + rs2.cnt));
			drawListsB.push_back(std::pair<uint, uint>(rs.offset_b, rs2.offset_b - rs.offset_b + rs2.cnt_b));
			di = -1;
		}
	}
	if (di > -1) {
		auto& rs = Particles::residueLists[di].residues[0];
		auto& rs2 = Particles::residueLists[Particles::residueListSz-1].residues[Particles::residueLists[Particles::residueListSz - 1].residueSz - 1];
		drawLists.push_back(std::pair<uint, uint>(rs.offset, rs2.offset - rs.offset + rs2.cnt));
		drawListsB.push_back(std::pair<uint, uint>(rs.offset_b, rs2.offset_b - rs.offset_b + rs2.cnt_b));
	}
	Scene::dirty = true;
}

void ParGraphics::Update() {
	if (!UI::editingText) {
		float s0 = rotScale;
		float rz0 = rotZ;
		float rw0 = rotW;
		Vec3 center0 = rotCenter;

		if (Input::mouse0) {
			if (Input::mouse0State == MOUSE_DOWN && VisSystem::InMainWin(Input::mousePos)) {
				dragging = true;
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
		else if (Input::mouseScroll != 0 && VisSystem::InMainWin(Input::mousePos)) {
			rotScale += 0.05f * Input::mouseScroll;
		}
		else {
			if (Input::KeyDown(Key_Escape)) {
				VisSystem::mouseMode = VIS_MOUSE_MODE::ROTATE;
			}
		}

		if (s0 != rotScale || rz0 != rotZ || rw0 != rotW || center0 != rotCenter) Scene::dirty = true;
	}
}

void ParGraphics::Rerender() {
	//*
	MVP::Switch(false);
	MVP::Clear();
	float csz = cos(-rotZ*deg2rad);
	float snz = sin(-rotZ*deg2rad);
	float csw = cos(rotW*deg2rad);
	float snw = sin(rotW*deg2rad);
	Mat4x4 mMatrix = Mat4x4(1, 0, 0, 0, 0, csw, snw, 0, 0, -snw, csw, 0, 0, 0, 0, 1) * Mat4x4(csz, 0, -snz, 0, 0, 1, 0, 0, snz, 0, csz, 0, 0, 0, 0, 1);
	MVP::Mul(mMatrix);
	float s = pow(2, rotScale);
	MVP::Scale(s, s, s);
	MVP::Translate(-rotCenter.x, -rotCenter.y, -rotCenter.z);

	if (dragging) {
		auto imvp = glm::inverse(MVP::projection() * MVP::modelview());
		scrX = imvp * Vec4(1, 0, 0, 0);
		scrY = imvp * Vec4(0, 1, 0, 0);
	}
	//*/
	/*
	MVP::Switch(true);
	float mww = pow(2, rotScale);
	//MVP::Scale(mww, mww*Display::height / Display::width, 1);
	MVP::Push();
	float csz = cos(-rotZ*deg2rad);
	float snz = sin(-rotZ*deg2rad);
	float csw = cos(rotW*deg2rad);
	float snw = sin(rotW*deg2rad);
	Mat4x4 mMatrix = Mat4x4(1, 0, 0, 0, 0, csw, snw, 0, 0, -snw, csw, 0, 0, 0, 0, 1) * Mat4x4(csz, 0, -snz, 0, 0, 1, 0, 0, snz, 0, csz, 0, 0, 0, 0, 1);
	MVP::Mul(mMatrix);
	MVP::Translate(-rotCenter.x, -rotCenter.y, -rotCenter.z);
	*/
	auto _mv = MVP::modelview();
	auto _p = MVP::projection();
	auto _cpos = ChokoLait::mainCamera->object->transform.position();
	auto _cfwd = ChokoLait::mainCamera->object->transform.forward();

	glUseProgram(parProg);
	glUniformMatrix4fv(parProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
	glUniformMatrix4fv(parProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
	glUniform3f(parProgLocs[2], _cpos.x, _cpos.y, _cpos.z);
	glUniform3f(parProgLocs[3], _cfwd.x, _cfwd.y, _cfwd.z);
	glUniform2f(parProgLocs[4], (float)Display::width, (float)Display::height);

	glBindVertexArray(Particles::posVao);
	for (auto& p : drawLists)
		glDrawArrays(GL_POINTS, p.first, p.second);

	/*
	glUseProgram(parConProg);
	glUniformMatrix4fv(parConProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
	glUniformMatrix4fv(parConProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
	glUniform3f(parConProgLocs[2], _cpos.x, _cpos.y, _cpos.z);
	glUniform3f(parConProgLocs[3], _cfwd.x, _cfwd.y, _cfwd.z);
	glUniform2f(parConProgLocs[4], (float)Display::width, (float)Display::height);
	glUniform1i(parConProgLocs[5], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);
	glUniform1i(parConProgLocs[6], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, Particles::connTexBuffer);

	glBindVertexArray(emptyVao);
	for (auto& p : drawListsB)
		glDrawArrays(GL_POINTS, p.first, p.second);
	*/
	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::Recolor() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, ChokoLait::mainCamera->d_colfbo);

	glUseProgram(colProg);
	glUniform1i(colProgLocs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->d_idTex);
	glUniform1i(colProgLocs[1], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->d_texs[2]);
	glUniform2f(colProgLocs[2], (float)Display::width, (float)Display::height);
	glUniform1i(colProgLocs[3], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
	glUniform1i(colProgLocs[4], 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(Camera::fullscreenVao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Camera::rectIdBuf);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);
	glBindVertexArray(0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ParGraphics::Reblit() {
	Recolor();
	BlitSky();
	if (hlIds.size())
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
	glUniform1i(reflProgLocs[4], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, cam->d_texs[2]);
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

	glBindVertexArray(Camera::fullscreenVao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Camera::rectIdBuf);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::BlitHl() {
	glUseProgram(selHlProg);

	glUniform2f(selHlProgLocs[0], (float)Display::width, (float)Display::height);
	glUniform1i(selHlProgLocs[1], hlIds[0]);
	glUniform1i(selHlProgLocs[2], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->d_idTex);
	glUniform3f(selHlProgLocs[3], 1.0f, 1.0f, 0.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glBindVertexArray(Camera::fullscreenVao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Camera::rectIdBuf);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::DrawMenu() {
	float s0 = rotScale;
	float rz0 = rotZ;
	float rw0 = rotW;
	Vec3 center0 = rotCenter;

	auto& expandPos = ParMenu::expandPos;
	auto& font = ParMenu::font;
	UI::Label(expandPos - 148, 3, 12, "Ambient", font, white());
	Engine::DrawQuad(expandPos - 149, 18, 148, 36, white(0.9f, 0.1f));
	UI::Label(expandPos - 147, 20, 12, "Strength", font, white());
	reflStr = Engine::DrawSliderFill(expandPos - 80, 19, 78, 16, 0, 2, reflStr, white(1, 0.5f), white());
	UI::Label(expandPos - 147, 37, 12, "Falloff", font, white());
	reflStrDecay = Engine::DrawSliderFill(expandPos - 80, 36, 78, 16, 0, 50, reflStrDecay, white(1, 0.5f), white());
	
	UI::Label(expandPos - 148, 54, 12, "Rim Light", font, white());
	Engine::DrawQuad(expandPos - 149, 68, 148, 38, white(0.9f, 0.1f));
	UI::Label(expandPos - 147, 71, 12, "Offset", font, white());
	rimOff = Engine::DrawSliderFill(expandPos - 80, 69, 78, 16, 0, 1, rimOff, white(1, 0.5f), white());
	UI::Label(expandPos - 147, 88, 12, "Strength", font, white());
	rimStr = Engine::DrawSliderFill(expandPos - 80, 88, 78, 16, 0, 5, rimStr, white(1, 0.5f), white());

	UI::Label(expandPos - 148, 105, 12, "Camera", font, white());
	Engine::DrawQuad(expandPos - 149, 121, 148, 106, white(0.9f, 0.1f));
	UI::Label(expandPos - 147, 122, 12, "Center X", font, white());
	UI::Label(expandPos - 147, 139, 12, "Center Y", font, white());
	UI::Label(expandPos - 147, 156, 12, "Center Z", font, white());
	rotCenter.x = TryParse(UI::EditText(expandPos - 80, 122, 78, 16, 12, Vec4(0.6f, 0.4f, 0.4f, 1), std::to_string(rotCenter.x), font, true, nullptr, white()), 0.0f);
	rotCenter.y = TryParse(UI::EditText(expandPos - 80, 139, 78, 16, 12, Vec4(0.4f, 0.6f, 0.4f, 1), std::to_string(rotCenter.y), font, true, nullptr, white()), 0.0f);
	rotCenter.z = TryParse(UI::EditText(expandPos - 80, 156, 78, 16, 12, Vec4(0.4f, 0.4f, 0.6f, 1), std::to_string(rotCenter.z), font, true, nullptr, white()), 0.0f);
	
	UI::Label(expandPos - 147, 174, 12, "Rotation W", font, white());
	UI::Label(expandPos - 147, 191, 12, "Rotation Y", font, white());
	rotW = TryParse(UI::EditText(expandPos - 80, 174, 78, 16, 12, Vec4(0.6f, 0.4f, 0.4f, 1), std::to_string(rotW), font, true, nullptr, white()), 0.0f);
	rotZ = TryParse(UI::EditText(expandPos - 80, 191, 78, 16, 12, Vec4(0.4f, 0.6f, 0.4f, 1), std::to_string(rotZ), font, true, nullptr, white()), 0.0f);
	
	UI::Label(expandPos - 147, 209, 12, "Scale", font, white());
	rotScale = TryParse(UI::EditText(expandPos - 80, 209, 78, 16, 12, Vec4(0.6f, 0.4f, 0.4f, 1), std::to_string(rotScale), font, true, nullptr, white()), 0.0f);

	rotW = Clamp<float>(rotW, -90, 90);
	rotZ = Repeat<float>(rotZ, 0, 360);

	if (s0 != rotScale || rz0 != rotZ || rw0 != rotW || center0 != rotCenter) Scene::dirty = true;
}