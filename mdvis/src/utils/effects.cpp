#include "ChokoLait.h"
#include "effects.h"
#include "res/shddata.h"

PROGDEF(Effects::blurProg)
PROGDEF(Effects::ssaoProg)
PROGDEF(Effects::ssaoProg2)
PROGDEF(Effects::glowProg)
PROGDEF(Effects::glowProg2)
PROGDEF(Effects::dofProg)

GLuint Effects::noiseTex;

void Effects::Init(EFF_ENABLE_MASK mask) {
	auto& vs = glsl::minVert;

	if (!!(mask & EFF_ENABLE_BLUR))
		_InitBlur(vs);
	if (!!(mask & EFF_ENABLE_GLOW))
		_InitGlow(vs);
	//if ((mask & EFF_ENABLE_FXAA) > 0x00ff)
	if ((mask & EFF_ENABLE_SSAO) > 0x00ff)
		_InitSSAO(vs);
	if (!!(mask & EFF_ENABLE_DOF))
		_InitDof(vs);
}

byte Effects::Blur(GLuint t1, GLuint t2, GLuint tx1, GLuint tx2, float rad, int w, int h) {
	glUseProgram(blurProg);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(blurProgLocs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1f(blurProgLocs[1], rad);
	glUniform2f(blurProgLocs[2], (float)w, (float)h);
	glUniform1f(blurProgLocs[3], 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, tx2);
	glUniform1f(blurProgLocs[3], 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t1);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindVertexArray(0);
	glUseProgram(0);
	return 2;
}

byte Effects::Glow(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, float off, float rad, float str, int w, int h) {
	glUseProgram(glowProg);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(glowProgLocs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1f(glowProgLocs[1], off);
	glUniform1f(glowProgLocs[2], rad);
	glUniform2f(glowProgLocs[3], (float)w, (float)h);
	glUniform1f(glowProgLocs[4], 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, tx2);
	glUniform1f(glowProgLocs[1], 0);
	glUniform1f(glowProgLocs[4], 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t3);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(glowProg2);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(glowProg2Locs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1i(glowProg2Locs[1], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tx3);
	glUniform1f(glowProg2Locs[2], str);
	glUniform2f(glowProg2Locs[3], (float)w, (float)h);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glUseProgram(0);
	return 1;
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
	glUniform1i(ssaoProgLocs[6], cnt);
	glUniformMatrix4fv(ssaoProgLocs[7], 1, GL_FALSE, glm::value_ptr(MVP::projection()));
	glUniformMatrix4fv(ssaoProgLocs[8], 1, GL_FALSE, glm::value_ptr(glm::inverse(MVP::projection())));

	glBindVertexArray(Camera::emptyVao);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t3);
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
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->texs.depthTex);
	glUniform1f(ssaoProg2Locs[3], str);
	glUniform2f(ssaoProg2Locs[4], (float)w, (float)h);

	glBindVertexArray(Camera::emptyVao);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindVertexArray(0);
	glUseProgram(0);
	return 1;
}

byte Effects::Dof(GLuint t1, GLuint t2, GLuint tx1, GLuint tx2, GLuint dph, float z, float f, float a, int n, int w, int h) {
	glUseProgram(dofProg);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(dofProgLocs[1], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dph);
	glUniform1f(dofProgLocs[2], z);
	glUniform1f(dofProgLocs[3], f);
	glUniform2f(dofProgLocs[5], (float)w, (float)h);
	glUniform1i(dofProgLocs[6], ChokoLait::mainCamera->ortographic);

	for (int i = 0; i < n; i++, a*=0.5) {
		glUniform1i(dofProgLocs[0], 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, !(i%2)? tx1 : tx2);
		glUniform1f(dofProgLocs[4], a);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, !(i%2)? t2 : t1);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	return n;
}

#define gu(i, nm) locs[i] = glGetUniformLocation(prog, #nm)

void Effects::_InitBlur(const std::string& vs) {
	auto& prog = blurProg = Shader::FromVF(vs, glsl::blurFrag);
	auto& locs = blurProgLocs;
	gu(0, mainTex);
	gu(1, mul);
	gu(2, screenSize);
	gu(3, isY);
}

void Effects::_InitGlow(const std::string& vs) {
	auto prog = glowProg = Shader::FromVF(vs, IO::GetText(IO::path + "glowFrag.glsl"));
	GLint* locs = glowProgLocs;
	gu(0, mainTex);
	gu(1, off);
	gu(2, rad);
	gu(3, screenSize);
	gu(4, isY);
	
	prog = glowProg2 = Shader::FromVF(vs, IO::GetText(IO::path + "glowFrag2.glsl"));
	locs = glowProg2Locs;
	gu(0, mainTex);
	gu(1, glowTex);
	gu(2, str);
	gu(3, screenSize);
}

void Effects::_InitSSAO(const std::string& vs) {
	auto prog = ssaoProg = Shader::FromVF(vs, glsl::ssaoFrag);
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
	
	prog = ssaoProg2 = Shader::FromVF(vs, glsl::ssaoFrag2);
	locs = ssaoProg2Locs;
	gu(0, tex1);
	gu(1, tex2);
	gu(2, dtex);
	gu(3, val);
	gu(4, screenSize);

	Vec3 noise[256];
	for (uint i = 0; i < 256; ++i) {
		noise[i] = (Vec3(Random::Range(0.5f, 1.f), Random::Value(), Random::Value()));
	}
	glGenTextures(1, &noiseTex);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_FLOAT, noise);
	SetTexParams<>(0, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Effects::_InitDof(const std::string& vs) {
	auto prog = dofProg = Shader::FromVF(vs, IO::GetText(IO::path + "dofFrag.glsl"));
	GLint* locs = dofProgLocs;
	gu(0, mainTex);
	gu(1, depthTex);
	gu(2, plane);
	gu(3, focal);
	gu(4, aperture);
	gu(5, screenSize);
	gu(6, isOrtho);
}