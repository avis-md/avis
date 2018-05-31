#include "effects.h"

void Effects::Init(EFF_ENABLE_MASK mask) {
    if (!!(mask & EFF_ENABLE_BLUR)) {
        
    }
    if (!!(mask & EFF_ENABLE_GLOW)) {
        
    }
    if (!!(mask & EFF_ENABLE_FXAA)) {
        
    }
    if (!!(mask & EFF_ENABLE_SSAO)) {
        
    }
}

byte Effects::SSAO(GLuint t1, GLuint t2, GLuint nrm, GLuint dph) {
    
}