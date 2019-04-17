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