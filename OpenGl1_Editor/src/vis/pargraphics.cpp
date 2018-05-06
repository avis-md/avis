#include "pargraphics.h"

Texture* ParGraphics::refl = nullptr;
float ParGraphics::reflStr = 1, ParGraphics::reflStrDecay = 2, ParGraphics::rimOff = 0.5f, ParGraphics::rimStr = 1;

GLuint ParGraphics::reflProg, ParGraphics::parProg, ParGraphics::parConProg;
GLint ParGraphics::reflProgLocs[] = {}, ParGraphics::parProgLocs[] = {}, ParGraphics::parConProgLocs[] = {};

GLuint ParGraphics::selHlProg, ParGraphics::colProg;
GLint ParGraphics::selHlProgLocs[] = {}, ParGraphics::colProgLocs[] = {};

std::vector<uint> ParGraphics::hlIds;
std::vector<std::pair<uint, uint>> ParGraphics::drawLists;

GLuint ParGraphics::emptyVao;

void ParGraphics::Init() {
	refl = new Texture(IO::path + "refl.png", true, TEX_FILTER_BILINEAR, 1, GL_REPEAT, GL_MIRRORED_REPEAT);
	reflProg = (new Shader(DefaultResources::GetStr("lightPassVert.txt"), IO::GetText("D:\\reflFrag.txt")))->pointer;
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

	selHlProg = (new Shader(DefaultResources::GetStr("lightPassVert.txt"), IO::GetText("D:\\selectorFrag.txt")))->pointer;
	selHlProgLocs[0] = glGetUniformLocation(selHlProg, "screenSize");
	selHlProgLocs[1] = glGetUniformLocation(selHlProg, "myId");
	selHlProgLocs[2] = glGetUniformLocation(selHlProg, "idTex");
	selHlProgLocs[3] = glGetUniformLocation(selHlProg, "hlCol");

	colProg = (new Shader(DefaultResources::GetStr("lightPassVert.txt"), IO::GetText("D:\\colorerFrag.txt")))->pointer;
	colProgLocs[0] = glGetUniformLocation(colProg, "idTex");
	colProgLocs[1] = glGetUniformLocation(colProg, "screenSize");
	colProgLocs[2] = glGetUniformLocation(colProg, "id2col");
	colProgLocs[3] = glGetUniformLocation(colProg, "colList");

	glGenVertexArrays(1, &emptyVao);

	hlIds.resize(1);
	ChokoLait::mainCamera->onBlit = Reblit;
}

void ParGraphics::UpdateDrawLists() {
	drawLists.clear();
	int di = -1;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& r = Particles::residueLists[i];
		if ((di == -1) && r.visible) di = i;
		else if ((di > -1) && !r.visible) {
			auto& rs = Particles::residueLists[di].residues[0];
			auto& rs2 = Particles::residueLists[i - 1].residues[Particles::residueLists[i - 1].residueSz-1];
			drawLists.push_back(std::pair<uint, uint>(rs.offset, rs2.offset - rs.offset + rs2.cnt));
			di = -1;
		}
	}
	if (di > -1) {
		auto& rs = Particles::residueLists[di].residues[0];
		auto& rs2 = Particles::residueLists[Particles::residueListSz-1].residues[Particles::residueLists[Particles::residueListSz - 1].residueSz - 1];
		drawLists.push_back(std::pair<uint, uint>(rs.offset, rs2.offset - rs.offset + rs2.cnt));
	}
	Scene::dirty = true;
}

void ParGraphics::Rerender() {
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
	glDrawArrays(GL_POINTS, 0, 5000000);

	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::Recolor() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindFramebuffer(GL_FRAMEBUFFER, ChokoLait::mainCamera->d_colfbo);

	glBindVertexArray(Camera::fullscreenVao);

	glUseProgram(colProg);
	glUniform1i(colProgLocs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->d_idTex);
	glUniform2f(colProgLocs[1], (float)Display::width, (float)Display::height);
	glUniform1i(colProgLocs[2], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, Particles::colorIdTexBuffer);
	glUniform1i(colProgLocs[3], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Particles::colorPalleteTex);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, Camera::fullscreenIndices);

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

	glBindVertexArray(Camera::fullscreenVao);

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
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, Camera::fullscreenIndices);

	glUseProgram(0);
	glBindVertexArray(0);
}

void ParGraphics::BlitHl() {
	glBindVertexArray(Camera::fullscreenVao);

	glUseProgram(selHlProg);

	glUniform2f(selHlProgLocs[0], (float)Display::width, (float)Display::height);
	glUniform1i(selHlProgLocs[1], hlIds[0]);
	glUniform1i(selHlProgLocs[2], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->d_idTex);
	glUniform3f(selHlProgLocs[3], 1.0f, 1.0f, 0.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, Camera::fullscreenIndices);

	glUseProgram(0);
	glBindVertexArray(0);
}