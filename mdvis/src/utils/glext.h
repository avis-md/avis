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

template <GLenum E = GL_ARRAY_BUFFER>
void SetGLSubBuf(GLuint buf, double* data, GLsizeiptr sz) {
    std::cout << "?";
    std::vector<float> tmp(sz);
#pragma omp parallel for
	for (int a = 0; a < sz; a++) {
		tmp[a] = (float)data[a];
	}
    SetGLSubBuf(buf, tmp.data(), sz);
}