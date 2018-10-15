#pragma once
#include "Engine.h"

template <typename T, GLenum E = GL_ARRAY_BUFFER>
void SetGLBuf(GLuint buf, T* data, GLsizeiptr sz, GLenum st = GL_STATIC_DRAW) {
    glBindBuffer(E, buf);
	glBufferData(E, sz * sizeof(T), data, st);
    glBindBuffer(E, 0);
}

template <typename T, GLenum E = GL_ARRAY_BUFFER>
void SetGLSubBuf(GLuint buf, T* data, GLsizeiptr sz) {
    glBindBuffer(E, buf);
	glBufferSubData(E, 0, sz * sizeof(T), data);
    glBindBuffer(E, 0);
}