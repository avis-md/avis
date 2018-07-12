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

class Effects {
public:
    static void Init(EFF_ENABLE_MASK mask);

	static byte Blur(GLuint t1, GLuint t2, GLuint tx1, GLuint tx2, float rad, int w, int h);
    static byte SSAO(GLuint t1, GLuint t2, GLuint t3, GLuint tx1, GLuint tx2, GLuint tx3, GLuint nrm, GLuint dph, float str, int cnt, float rad, float blr, int w, int h);

protected:
    static GLuint blurProg, ssaoProg, ssaoProg2;
    static GLint blurProgLocs[4], ssaoProgLocs[10], ssaoProg2Locs[5];
	static GLuint noiseTex;

    static void _InitBlur(const string& vs), _InitSSAO(const string& vs);
};