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

#include "ChokoLait.h"
#include "effects.h"
#include "res/shd/minVert.h"
#include "res/shd/blurFrag.h"
#include "res/shd/glowFrag.h"
#include "res/shd/glowFrag2.h"
#include "res/shd/ssaoFrag.h"
#include "res/shd/ssaoFrag2.h"
#include "res/shd/dofFrag.h"
#include "res/shd/fxaaFrag.h"

Shader Effects::blurProg;
Shader Effects::ssaoProg;
Shader Effects::ssaoProg2;
Shader Effects::glowProg;
Shader Effects::glowProg2;
Shader Effects::dofProg;
Shader Effects::fxaaProg;

GLuint Effects::noiseTex;

void Effects::Init(EFF_ENABLE_MASK mask) {
	auto& vs = glsl::minVert;

	if (!!(mask & EFF_ENABLE_BLUR))
		_InitBlur(vs);
	if (!!(mask & EFF_ENABLE_GLOW))
		_InitGlow(vs);
	if ((mask & EFF_ENABLE_FXAA) > 0x00ff)
		_InitFXAA(vs);
	if ((mask & EFF_ENABLE_SSAO) > 0x00ff)
		_InitSSAO(vs);
	if (!!(mask & EFF_ENABLE_DOF))
		_InitDof(vs);

}

bool Effects::Blur(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, float rad, int w, int h) {
	glUseProgram(blurProg);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(blurProg.Loc(0), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1f(blurProg.Loc(1), rad);
	glUniform2f(blurProg.Loc(2), (float)w, (float)h);
	glUniform1f(blurProg.Loc(3), 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, tx2);
	glUniform1f(blurProg.Loc(3), 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t3);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindVertexArray(0);
	glUseProgram(0);
	return false;
}

bool Effects::Glow(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, float off, float rad, float str, int w, int h) {
	glUseProgram(glowProg);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(glowProg.Loc(0), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1f(glowProg.Loc(1), off);
	glUniform1f(glowProg.Loc(2), rad);
	glUniform2f(glowProg.Loc(3), (float)w, (float)h);
	glUniform1f(glowProg.Loc(4), 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, tx2);
	glUniform1f(glowProg.Loc(1), 0);
	glUniform1f(glowProg.Loc(4), 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t3);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(glowProg2);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(glowProg2.Loc(0), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1i(glowProg2.Loc(1), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tx3);
	glUniform1f(glowProg2.Loc(2), str);
	glUniform2f(glowProg2.Loc(3), (float)w, (float)h);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glUseProgram(0);
	return true;
}

bool Effects::SSAO(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, GLuint nrm, GLuint dph, float str, int cnt, float rad, float blr, int w, int h) {
	glUseProgram(ssaoProg);
	glUniform1i(ssaoProg.Loc(0), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, nrm);
	glUniform1i(ssaoProg.Loc(1), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dph);
	glUniform1i(ssaoProg.Loc(2), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glUniform2f(ssaoProg.Loc(3), (float)w, (float)h);
	glUniform1f(ssaoProg.Loc(4), rad);
	glUniform1i(ssaoProg.Loc(6), cnt);
	glUniformMatrix4fv(ssaoProg.Loc(7), 1, GL_FALSE, glm::value_ptr(MVP::projection()));
	glUniformMatrix4fv(ssaoProg.Loc(8), 1, GL_FALSE, glm::value_ptr(glm::inverse(MVP::projection())));

	glBindVertexArray(Camera::emptyVao);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t3);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (blr > 0) Blur(t3, t2, t3, tx3, tx2, blr / 20, w, h);
	
	glUseProgram(ssaoProg2);
	glUniform1i(ssaoProg2.Loc(0), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform1i(ssaoProg2.Loc(1), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tx3);
	glUniform1i(ssaoProg2.Loc(2), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ChokoLait::mainCamera->texs.depthTex);
	glUniform1f(ssaoProg2.Loc(3), str);
	glUniform2f(ssaoProg2.Loc(4), (float)w, (float)h);

	glBindVertexArray(Camera::emptyVao);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindVertexArray(0);
	glUseProgram(0);
	return true;
}

bool Effects::Dof(GLuint t1, GLuint t2, GLuint tx1, GLuint tx2, GLuint dph, float z, float f, float a, int n, int w, int h) {
	glUseProgram(dofProg);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(dofProg.Loc(1), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dph);
	glUniform1f(dofProg.Loc(2), z);
	glUniform1f(dofProg.Loc(3), f);
	glUniform2f(dofProg.Loc(5), (float)w, (float)h);
	glUniform1i(dofProg.Loc(6), ChokoLait::mainCamera->ortographic);

	for (int i = 0; i < n && a >= 0.5f; i++, a*=0.5) {
		glUniform1i(dofProg.Loc(0), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, !(i%2)? tx1 : tx2);
		glUniform1f(dofProg.Loc(4), a);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, !(i%2)? t2 : t1);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	return (n % 2 == 1);
}

bool Effects::FXAA(GLuint t2, GLuint tx1, float spn, float max, float cut, int w, int h) {
	glUseProgram(fxaaProg);
	glBindVertexArray(Camera::emptyVao);
	glUniform1i(fxaaProg.Loc(0), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tx1);
	glUniform2f(fxaaProg.Loc(1), (float)w, (float)h);
	glUniform1f(fxaaProg.Loc(2), spn);
	glUniform1f(fxaaProg.Loc(3), max);
	glUniform1f(fxaaProg.Loc(4), cut);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, t2);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	return true;
}

void Effects::_InitBlur(const std::string& vs) {
	(blurProg = Shader::FromVF(vs, glsl::blurFrag))
		.AddUniform("mainTex")
		.AddUniform("mul")
		.AddUniform("screenSize")
		.AddUniform("isY");
}

void Effects::_InitGlow(const std::string& vs) {
	(glowProg = Shader::FromVF(vs, glsl::glowFrag))
		.AddUniform("mainTex")
		.AddUniform("off")
		.AddUniform("rad")
		.AddUniform("screenSize")
		.AddUniform("isY");
	
	(glowProg2 = Shader::FromVF(vs, glsl::glowFrag2))
		.AddUniform("mainTex")
		.AddUniform("glowTex")
		.AddUniform("str")
		.AddUniform("screenSize");
}

void Effects::_InitSSAO(const std::string& vs) {
	(ssaoProg = Shader::FromVF(vs, glsl::ssaoFrag))
		.AddUniform("normTex")
		.AddUniform("depthTex")
		.AddUniform("noiseTex")
		.AddUniform("screenSize")
		.AddUniform("radius")
		.AddUniform("strength")//
		.AddUniform("samples")
		.AddUniform("_P")
		.AddUniform("_IP");
	
	(ssaoProg2 = Shader::FromVF(vs, glsl::ssaoFrag2))
		.AddUniform("tex1")
		.AddUniform("tex2")
		.AddUniform("dtex")
		.AddUniform("val")
		.AddUniform("screenSize");

	Vec3 noise[256];
	for (uint i = 0; i < 256; ++i) {
		noise[i] = (Vec3(Random::Range(0.5f, 1.f), Random::Value(), Random::Value()));
	}
	glGenTextures(1, &noiseTex);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 16, 16, 0, GL_RGB, GL_FLOAT, noise);
	SetTexParams<>(0, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Effects::_InitDof(const std::string& vs) {
	(dofProg = Shader::FromVF(vs, glsl::dofFrag))
		.AddUniform("mainTex")
		.AddUniform("depthTex")
		.AddUniform("plane")
		.AddUniform("focal")
		.AddUniform("aperture")
		.AddUniform("screenSize")
		.AddUniform("isOrtho");
}

void Effects::_InitFXAA(const std::string& vs) {
	(fxaaProg = Shader::FromVF(vs, glsl::fxaaFrag))
		.AddUniform("tex")
		.AddUniform("screenSize")
		.AddUniform("spanMax")
		.AddUniform("reduceMul")
		.AddUniform("reduceCutoff");
}