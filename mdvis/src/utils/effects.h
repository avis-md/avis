#pragma once
#include "Engine.h"

typedef ushort EFF_ENABLE_MASK;
#define EFF_ENABLE_BLUR 0x0001
#define EFF_ENABLE_GLOW 0x0002
#define EFF_ENABLE_FXAA 0x0100
#define EFF_ENABLE_SSAO 0x0201

class Effects {
public:
    static void Init(EFF_ENABLE_MASK mask);

    static byte SSAO(GLuint t1, GLuint t2, GLuint nrm, GLuint dph);

protected:
    GLuint ssaoProg;
    GLint ssaoProgLocs[8];

    void _InitSSAO();
};