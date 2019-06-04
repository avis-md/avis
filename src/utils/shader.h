#pragma once
#include "Engine.h"
#include "utils/refcnt.h"

class Shader : public RefCnt {
public:
	Shader(GLuint ptr = 0) : pointer(ptr), pointers(1, ptr), options({}) {}
	Shader(const std::string& vert, const std::string& frag);
	~Shader() { CheckUniqueRef(); }

	operator bool() const {
		return !!pointer;
	}
	operator GLuint() const {
		return pointer;
	}

	Shader& AddUniform(const std::string& s);
	Shader& AddUniforms(std::initializer_list<const std::string> ss);

	void SetOptions(const std::initializer_list<std::string>& nms);
	void SetOption(const std::string& nm, bool on);

	void Bind();
	static void Unbind();

	const GLint Loc(int);

	static bool LoadShader(GLenum shaderType, std::string source, GLuint& shader, std::string* err = nullptr);

	static GLuint FromVF(const std::string& vert, const std::string& frag);
	static GLuint FromF(GLuint vert, const std::string& frag);
	
	static bool LinkShader(GLuint prog);

protected:
	GLuint pointer;

	std::vector<GLuint> pointers;
	std::vector<std::pair<std::string, bool>> options;

	std::vector<GLint> uniforms;

	void UpdatePointer();

	void DestroyRef() override;
};