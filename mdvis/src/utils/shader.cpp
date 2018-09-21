#include "shader.h"

bool Shader::LoadShader(GLenum shaderType, std::string source, GLuint& shader, std::string* err) {

	int compile_result = 0;

	shader = glCreateShader(shaderType);
#ifdef PLATFORM_WIN
	const char *shader_code_ptr = source.c_str();
	const int shader_code_size = source.size();
	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
#else
	const char *strings[1] = { NULL };
	strings[0] = source.c_str();
	glShaderSource(shader, 1, strings, NULL);
#endif
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);

	//check for errors
	if (!compile_result)
	{
		if (err) {
			int info_log_length = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
			if (!!info_log_length) {
				std::vector<char> shader_log(info_log_length);
				glGetShaderInfoLog(shader, info_log_length, NULL, &shader_log[0]);
				shader = 0;
				*err += std::string(&shader_log[0]);
				//LOGI(&shader_log[0]);
				Debug::Error("Shader", *err);
			}
		}
		glDeleteShader(shader);
		return false;
	}
	//std::std::cout << "shader compiled" << std::endl;
	return true;
}

GLuint Shader::FromVF(const std::string& vert, const std::string& frag) {
	GLuint vertex_shader;
	std::string err = "";
	if (vert == "" || frag == "") {
		Debug::Error("Shader Compiler", "vert or frag is empty!");
	}
	
	if (!LoadShader(GL_VERTEX_SHADER, vert, vertex_shader, &err)) {
		Debug::Error("Shader Compiler", "Vert error: " + err);
		abort();
		return 0;
	}
	
	auto pointer = FromF(vertex_shader, frag);

	glDeleteShader(vertex_shader);
	return pointer;
}

GLuint Shader::FromF(GLuint vert, const std::string& frag) {
	GLuint vertex_shader = vert, fragment_shader;
	std::string err;

	if (!vert || frag == "") {
		Debug::Error("Shader Compiler", "vert or frag is empty!");
	}

	if (!LoadShader(GL_FRAGMENT_SHADER, frag, fragment_shader, &err)) {
		Debug::Error("Shader Compiler", "Frag error: " + err);
		abort();
		return 0;
	}

	GLuint pointer = glCreateProgram();
	glAttachShader(pointer, vertex_shader);
	glAttachShader(pointer, fragment_shader);

	int link_result = 0;

	glLinkProgram(pointer);
	glGetProgramiv(pointer, GL_LINK_STATUS, &link_result);
	if (link_result == GL_FALSE)
	{
		int info_log_length = 0;
		glGetProgramiv(pointer, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(pointer, info_log_length, NULL, &program_log[0]);
		Debug::Error("Shader", "Link error: " + std::string(&program_log[0]));
		glDeleteProgram(pointer);
		return 0;
	}

	glDetachShader(pointer, vertex_shader);
	glDetachShader(pointer, fragment_shader);
	glDeleteShader(fragment_shader);
	return pointer;
}