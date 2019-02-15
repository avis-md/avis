#pragma once
#include "Engine.h"
#include "utils/refcnt.h"

class Shader : public RefCnt {
public:
	Shader(GLuint ptr = 0) : pointer(ptr) {}
	~Shader() { CheckUniqueRef(); }

	operator bool() const {
		return !!pointer;
	}
	operator GLuint() const {
		return pointer;
	}

	Shader& AddUniform(const std::string& s);

	void Bind();
	static void Unbind();

	const GLint Loc(int);

	static bool LoadShader(GLenum shaderType, std::string source, GLuint& shader, std::string* err = nullptr);

	static GLuint FromVF(const std::string& vert, const std::string& frag);
	static GLuint FromF(GLuint vert, const std::string& frag);

protected:
	GLuint pointer;

	std::vector<GLint> uniforms;

	void DestroyRef() override;
};