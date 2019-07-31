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

#include "shader.h"


Shader::Shader(const std::string& vert, const std::string& frag) {
	uint vers = 1;
	std::string s;
	std::istringstream vstrm(vert);
	while (std::getline(vstrm, s)) {
		if (s[0] != '#') continue;
		auto ss = string_split(s, ' ', true);
		if (ss[0] == "#pragma") {
			if (ss[1] == "option") {
				options.push_back(std::make_pair(ss[2], false));
				vers *= 2;
			}
		}
	}
	std::istringstream fstrm(frag);
	while (std::getline(fstrm, s)) {
		if (s[0] == '#') {
			auto ss = string_split(s, ' ', true);
			if (ss[0] == "#pragma") {
				if (ss[1] == "option") {
					if (std::find_if(options.begin(), options.end(), [&](const std::pair<std::string, bool>& o) {
						return o.first == ss[2];
					}) == options.end()) {
						options.push_back(std::make_pair(ss[2], false));
						vers *= 2;
					}
				}
			}
		}
	}

	for (uint v = 0; v < vers; v++) {
		std::string prep = "#version 330 core\n";
		for (int a = 0; a < options.size(); a++) {
			if (!!(v & (1 << a))) {
				prep += "#define " + options[a].first + "\n";
			}
		}
		pointers.push_back(FromVF(prep + vert, prep + frag));
	}
	pointer = pointers[0];
}

Shader& Shader::AddUniform(const std::string& s) {
	uniforms.push_back(glGetUniformLocation(pointer, s.c_str()));
	return *this;
}

Shader& Shader::AddUniforms(std::initializer_list<const std::string> ss) {
	for (auto& s : ss)
		AddUniform(s);
	return *this;
}

void Shader::SetOptions(const std::initializer_list<std::string>& nms) {
	for (auto& v : options) {
		v.second = std::find(nms.begin(), nms.end(), v.first) != nms.end();
	}
	UpdatePointer();
}

void Shader::SetOption(const std::string& nm, bool on) {
	for (auto& v : options) {
		if (v.first == nm) {
			v.second = on;
			break;
		}
	}
	UpdatePointer();
}

void Shader::Bind() {
	glUseProgram(pointer);
}

void Shader::Unbind() {
	glUseProgram(0);
}

GLint Shader::Loc(int i) {
	return uniforms[i];
}

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
	//std::cout << "shader compiled" << std::endl;
	return true;
}

GLuint Shader::FromVF(const std::string& vert, const std::string& frag) {
	GLuint vertex_shader;
	std::string err = "";
	if (vert == "" || frag == "") {
		Debug::Error("Shader Compiler", "vert or frag is empty!");
		return 0;
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
		return 0;
	}

	if (!LoadShader(GL_FRAGMENT_SHADER, frag, fragment_shader, &err)) {
		Debug::Error("Shader Compiler", "Frag error: " + err);
		abort();
		return 0;
	}

	GLuint pointer = glCreateProgram();
	glAttachShader(pointer, vertex_shader);
	glAttachShader(pointer, fragment_shader);

	if (!LinkShader(pointer)) {
		glDeleteProgram(pointer);
		return 0;
	}

	glDetachShader(pointer, vertex_shader);
	glDetachShader(pointer, fragment_shader);
	glDeleteShader(fragment_shader);
	return pointer;
}

bool Shader::LinkShader(GLuint pointer) {
	int link_result = 0;
	glLinkProgram(pointer);
	glGetProgramiv(pointer, GL_LINK_STATUS, &link_result);
	if (link_result == GL_FALSE) {
		int info_log_length = 0;
		glGetProgramiv(pointer, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(pointer, info_log_length, NULL, &program_log[0]);
		Debug::Error("Shader", "Link error: " + std::string(&program_log[0]));
		return false;
	}
	return true;
}

void Shader::UpdatePointer() {
	uint i = 0, j = 0;
	for (auto& v : options) {
		if (v.second)
			i |= (1 << j);
		j++;
	}
	pointer = pointers[i];
}

void Shader::DestroyRef() {
	if (!!pointer)
		glDeleteProgram(pointer);
}