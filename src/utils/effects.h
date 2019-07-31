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

#pragma once
#include "ChokoLait.h"

enum class EFF_TYPE {
	BLUR,
	GLOW,
	FXAA,
	SSAO
};

typedef ushort EFF_ENABLE_MASK;
#define EFF_ENABLE_BLUR 0x0001
#define EFF_ENABLE_GLOW 0x0002
#define EFF_ENABLE_FXAA 0x0100
#define EFF_ENABLE_SSAO 0x0201
#define EFF_ENABLE_DOF 0x0400

class Effects {
public:
	static void Init(EFF_ENABLE_MASK mask);

	static bool Blur(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, float rad, int w, int h);
	static bool Glow(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, float off, float rad, float str, int w, int h);
	static bool SSAO(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, GLuint nrm, GLuint dph, float str, int cnt, float rad, float blr, int w, int h);
	static bool Dof(GLuint t1, GLuint t2, GLuint tx1, GLuint tx2, GLuint dph, float z, float f, float a, int n, int w, int h);
	static bool FXAA(GLuint t2, GLuint tx1, float spn, float max, float cut, int w, int h);
protected:
	static Shader blurProg;
	static Shader ssaoProg;
	static Shader ssaoProg2;
	static Shader glowProg;
	static Shader glowProg2;
	static Shader dofProg;
	static Shader fxaaProg;

	static GLuint noiseTex;

	static void _InitBlur(const std::string& vs);
	static void _InitGlow(const std::string& vs);
	static void _InitSSAO(const std::string& vs);
	static void _InitDof(const std::string& vs);
	static void _InitFXAA(const std::string& vs);
};