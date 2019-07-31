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
    std::vector<float> tmp(sz);
#pragma omp parallel for
	for (int a = 0; a < sz; a++) {
		tmp[a] = (float)data[a];
	}
    SetGLSubBuf(buf, tmp.data(), sz);
}