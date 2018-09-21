#pragma once
#define PROGDEF(nm) GLuint nm; GLint nm ## Locs[] = {};
#define PROGDEF_H(nm, c) static GLuint nm; static GLint nm ## Locs[c];

#include "Engine.h"

class Shader {
public:
	static bool LoadShader(GLenum shaderType, std::string source, GLuint& shader, std::string* err = nullptr);

	static GLuint FromVF(const std::string& vert, const std::string& frag);
	static GLuint FromF(GLuint vert, const std::string& frag);
};