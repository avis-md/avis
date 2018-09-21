#include "effects.h"
#include "res/shddata.h"

GLuint Effects::blurProg, Effects::ssaoProg, Effects::ssaoProg2;
GLint Effects::blurProgLocs[], Effects::ssaoProgLocs[], Effects::ssaoProg2Locs[];
GLuint Effects::noiseTex;

void Effects::Init(EFF_ENABLE_MASK mask) {
	auto vs = glsl::minVert;

	if (!!(mask & EFF_ENABLE_BLUR))
		_InitBlur(vs);
	//if (!!(mask & EFF_ENABLE_GLOW))
	//if ((mask & EFF_ENABLE_FXAA) > 0x00ff)
	if ((mask & EFF_ENABLE_SSAO) > 0x00ff)
		_InitSSAO(vs);
}

byte Effects::Blur(GLuint t1, GLuint t2, GLuint tx1, GLuint tx2, float rad, int w, int h) {
	glUseProgram(blurProg);
	glBindVertexArray(Camera::emptyVao);
	//glBindVertexArray(Camera::fullscreenVao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Camera::rectIdBuf);
	glUniform1i(blurProgLocs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1f(blurProgLocs[1], rad);
	glUniform2f(blurProgLocs[2], (float)w, (float)h);
	glUniform1f(blurProgLocs[3], 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, tx2);
	glUniform1f(blurProgLocs[3], 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t1);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	return 2;
}

byte Effects::SSAO(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, GLuint nrm, GLuint dph, float str, int cnt, float rad, float blr, int w, int h) {
	glUseProgram(ssaoProg);
	glUniform1i(ssaoProgLocs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, nrm);
	glUniform1i(ssaoProgLocs[1], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dph);
	glUniform1i(ssaoProgLocs[2], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glUniform2f(ssaoProgLocs[3], (float)w, (float)h);
	glUniform1f(ssaoProgLocs[4], rad);
	//glUniform1f(ssaoProgLocs[5], 1);
	glUniform1i(ssaoProgLocs[6], cnt);
	glUniformMatrix4fv(ssaoProgLocs[7], 1, GL_FALSE, glm::value_ptr(MVP::projection()));
	glUniformMatrix4fv(ssaoProgLocs[8], 1, GL_FALSE, glm::value_ptr(glm::inverse(MVP::projection())));

	glBindVertexArray(Camera::emptyVao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Camera::rectIdBuf);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t3);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (blr > 0) Blur(t3, t2, tx3, tx2, blr / 20, w, h);
	
	glUseProgram(ssaoProg2);
	glUniform1i(ssaoProg2Locs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1i(ssaoProg2Locs[1], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tx3);
	glUniform1i(ssaoProg2Locs[2], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->d_depthTex);
	glUniform1f(ssaoProg2Locs[3], str);
	glUniform2f(ssaoProg2Locs[4], (float)w, (float)h);

	glBindVertexArray(Camera::emptyVao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Camera::rectIdBuf);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	return 1;
}

#define gu(i, nm) locs[i] = glGetUniformLocation(prog, #nm)

void Effects::_InitBlur(const std::string& vs) {
	auto& prog = blurProg = Shader::FromVF(vs, IO::GetText(IO::path + "blurFrag.txt"));
	auto& locs = blurProgLocs;
	gu(0, mainTex);
	gu(1, mul);
	gu(2, screenSize);
	gu(3, isY);
}

void Effects::_InitSSAO(const std::string& vs) {
	auto prog = ssaoProg = Shader::FromVF(vs, IO::GetText(IO::path + "ssaoFrag.txt"));
	GLint* locs = ssaoProgLocs;
	gu(0, normTex);
	gu(1, depthTex);
	gu(2, noiseTex);
	gu(3, screenSize);
	gu(4, radius);
	//gu(5, strength);
	gu(6, samples);
	gu(7, _P);
	gu(8, _IP);
	
	prog = ssaoProg2 = Shader::FromVF(vs, IO::GetText(IO::path + "ssaoFrag2.txt"));
	locs = ssaoProg2Locs;
	gu(0, tex1);
	gu(1, tex2);
	gu(2, depth);
	gu(3, val);
	gu(4, screenSize);

	Vec3 noise[256];
	for (uint i = 0; i < 256; i++) {
		noise[i] = (Vec3(Random::Range(0.5f, 1.0f), Random::Value(), Random::Value()));
	}
	glGenTextures(1, &noiseTex);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_FLOAT, noise);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}