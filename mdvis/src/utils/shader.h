#pragma once
#define PROGDEF(nm) GLuint nm; GLint nm ## Locs[] = {};
#define PROGDEF_H(nm, c) static GLuint nm; static GLint nm ## Locs[c];

#include "Engine.h"

class Shader {
public:
	static bool LoadShader(GLenum shaderType, string source, GLuint& shader, string* err = nullptr);

	static GLuint FromVF(const string& vert, const string& frag);
	static GLuint FromF(GLuint vert, const string& frag);
};