#include "cubemarcher.h"
#include "res/shddata.h"

GLuint CubeMarcher::prog = 0;

CubeMarcher::CubeMarcher(Texture3D* tex) : tex(tex) {

}

void CubeMarcher::Init() {
	GLuint geometry_shader, vertex_shader, fragment_shader;
	string err;

	if (!Shader::LoadShader(GL_VERTEX_SHADER, IO::GetText(IO::path + "/mcVert.txt"), vertex_shader, &err)) {
		Debug::Error("Shader Compiler", "Vert error: " + err);
	}
	if (!Shader::LoadShader(GL_GEOMETRY_SHADER, IO::GetText(IO::path + "/geo.txt"), geometry_shader, &err)) {
		Debug::Error("Shader Compiler", "Geom error: " + err);
	}
	if (!Shader::LoadShader(GL_FRAGMENT_SHADER, glsl::coreFrag3, fragment_shader, &err)) {
		Debug::Error("Shader Compiler", "Frag error: " + err);
	}

	prog = glCreateProgram();
	glAttachShader(prog, vertex_shader);
	glAttachShader(prog, geometry_shader);
	glAttachShader(prog, fragment_shader);

	int link_result = 0;

	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &link_result);
	if (link_result == GL_FALSE)
	{
		int info_log_length = 0;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(prog, info_log_length, NULL, &program_log[0]);
		Debug::Error("Shader", "Link error: " + string(&program_log[0]));
		glDeleteProgram(prog);
	}

	glDetachShader(prog, vertex_shader);
	glDetachShader(prog, geometry_shader);
	glDetachShader(prog, fragment_shader);
	glDeleteShader(geometry_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

void CubeMarcher::Draw(const Mat4x4& _mvp) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUseProgram(prog);
	glBindVertexArray(Camera::emptyVao);
	auto sz = glGetUniformLocation(prog, "sz");
	auto mvp = glGetUniformLocation(prog, "_MVP");
	auto cl = glGetUniformLocation(prog, "col");
	glUniform3i(sz, 5, 5, 5);
	glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(_mvp));
	glUniform4f(cl, 1, 0, 0, 1);
	glDrawArrays(GL_POINTS, 0, 125);
	glBindVertexArray(0);
	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}