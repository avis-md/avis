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

	static byte Blur(GLuint t1, GLuint t2, GLuint tx1, GLuint tx2, float rad, int w, int h);
	static byte Glow(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, float off, float rad, float str, int w, int h);
	static byte SSAO(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, GLuint nrm, GLuint dph, float str, int cnt, float rad, float blr, int w, int h);
	static byte Dof(GLuint t1, GLuint t2, GLuint tx1, GLuint tx2, GLuint dph, float z, float f, float a, int n, int w, int h);
protected:
	PROGDEF_H(blurProg, 5)
	PROGDEF_H(ssaoProg, 10)
	PROGDEF_H(ssaoProg2, 5)
	PROGDEF_H(glowProg, 10)
	PROGDEF_H(glowProg2, 5)
	PROGDEF_H(dofProg, 10)

	static GLuint noiseTex;

	static void _InitBlur(const std::string& vs);
	static void _InitGlow(const std::string& vs);
	static void _InitSSAO(const std::string& vs);
	static void _InitDof(const std::string& vs);
};