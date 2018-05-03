#include "pargraphics.h"

Texture* ParGraphics::refl = nullptr;

GLuint ParGraphics::reflProg, ParGraphics::parProg, ParGraphics::parConProg;
GLint ParGraphics::reflProgLocs[] = {}, ParGraphics::parProgLocs[] = {}, ParGraphics::parConProgLocs[] = {};

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

	glGenVertexArrays(1, &emptyVao);
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
	glUniform2f(parProgLocs[4], Display::width, Display::height);

	glBindVertexArray(Particles::posVao);
	glDrawArrays(GL_POINTS, 0, 10000000);

	/*
	glUseProgram(parConProg);
	glUniformMatrix4fv(parConProgLocs[0], 1, GL_FALSE, glm::value_ptr(_mv));
	glUniformMatrix4fv(parConProgLocs[1], 1, GL_FALSE, glm::value_ptr(_p));
	glUniform3f(parConProgLocs[2], _cpos.x, _cpos.y, _cpos.z);
	glUniform3f(parConProgLocs[3], _cfwd.x, _cfwd.y, _cfwd.z);
	glUniform2f(parConProgLocs[4], Display::width, Display::height);
	glUniform1i(parConProgLocs[5], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, Particles::posTexBuffer);

	glBindVertexArray(emptyVao);
	glDrawArrays(GL_POINTS, 0, 5000000);
	*/
	glBindVertexArray(0);
	glUseProgram(0);
}

void ParGraphics::Recolor() {

}

void ParGraphics::Reblit() {
	auto _p = MVP::projection();
	auto cam = ChokoLait::mainCamera().get();
	
	glBindVertexArray(Camera::fullscreenVao);
	/*
	uniform mat4 _IP;
	uniform vec2 screenSize;
	uniform sampler2D inColor;
	uniform sampler2D inNormal;
	uniform sampler2D inSpec;
	uniform sampler2D inDepth;

	uniform sampler2D inSky;
	uniform float skyStrength;
	*/
	glUseProgram(reflProg);
	glUniformMatrix4fv(reflProgLocs[0], 1, GL_FALSE, glm::value_ptr(glm::inverse(_p)));
	glUniform2f(reflProgLocs[1], Display::width, Display::height);
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

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, Camera::fullscreenIndices);

	glUseProgram(0);
	glBindVertexArray(0);
}

void ParGraphics::BlitSky() {

}