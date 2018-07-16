#include "Engine.h"
#include <limits>

IShaderBuffer::IShaderBuffer(uint size, void* data, uint padding, uint stride) : size(size) {
	glGenBuffers(1, &pointer);
	glBindBuffer(GL_UNIFORM_BUFFER, pointer);
	glBufferData(GL_UNIFORM_BUFFER, size, (!!padding) ? nullptr : data, GL_DYNAMIC_READ);
	if (!!padding) Set(data, padding, stride);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
IShaderBuffer::~IShaderBuffer() {
	glDeleteBuffers(1, &pointer);
}

void IShaderBuffer::Set(void* data, uint padding, uint stride) {
	if (!data) {
		Debug::Warning("ShaderBuffer", "Set: Buffer is null!");
	}
	glBindBuffer(GL_UNIFORM_BUFFER, pointer);
	void* tar = (void*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	if (!tar) {
		Debug::Warning("ShaderBuffer", "Set: Unable to map buffer!");
	}
	if (!padding) memcpy(tar, data, size);
	else {
		for (uint a = 0; a*padding < size; a++) {
			memcpy((void*)((char*)tar + a*padding), (void*)((char*)data + a*stride), stride);
		}
	}
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


bool Shader::LoadShader(GLenum shaderType, string source, GLuint& shader, string* err) {

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
				*err += string(&shader_log[0]);
				//LOGI(&shader_log[0]);
				Debug::Error("Shader", *err);
			}
		}
		glDeleteShader(shader);
		//LOGI("fail!");
		//abort();
		return false;
	}
	//std::std::cout << "shader compiled" << std::endl;
	return true;
}

Shader::Shader(string p) : AssetObject(ASSETTYPE_SHADER) {
	std::ifstream strm(p.c_str(), std::ios::in | std::ios::binary);
	std::stringstream strm2; //15% faster
	strm2 << strm.rdbuf();
	std::istream stream(strm2.rdbuf());
	string vertex_shader_code = "";
	string fragment_shader_code = "";
	//std::ifstream stream(p.c_str());
	if (!stream.good()) {
		std::cout << "shader not found!" << std::endl;
		return;
	}
	char* c = new char[4];
	stream.read(c, 3);
	c[3] = char0;
	string ss(c);
	if (string(c) != "KTS") {
		std::cerr << "file not supported" << std::endl;
		return;
	}

	int vs;
	_Strm2Val(stream, vs);

	char type;
	char* nmm = new char[100];
	for (int r = 0; r < vs; r++) {
		stream.get(type);
		byte bb = type;
		vars.push_back(new ShaderVariable());
		vars[r]->type = (SHADER_VARTYPE)bb;
		switch (bb) {
		case SHADER_INT:
			_Strm2Val(stream, vars[r]->min);
			_Strm2Val(stream, vars[r]->max);
			_Strm2Val(stream, vars[r]->val.i);
			break;
		case SHADER_FLOAT:
			_Strm2Val(stream, vars[r]->min);
			_Strm2Val(stream, vars[r]->max);
			_Strm2Val(stream, vars[r]->val.x);
			break;
		case SHADER_SAMPLER:
			byte bbb;
			_Strm2Val(stream, bbb);
			vars[r]->def.i = bbb;
			vars[r]->val.i = -1;
			break;
		}
		stream.getline(nmm, 100, char0);
		vars[r]->name += string(nmm);
	}
	stream.get(type);
	if ((byte)type != 0xff)
		return;

	int i;
	_Strm2Val(stream, i);
	char* cc = new char[i + 1];
	stream.read(cc, i);
	cc[i] = char0;
	vertex_shader_code = string(cc);
	delete[](cc);

	stream.get(type);
	if ((byte)type != 0x00)
		return;

	_Strm2Val(stream, i);
	cc = new char[i + 1];
	stream.read(cc, i);
	cc[i] = char0;
	fragment_shader_code = string(cc);
	delete[](cc);
	//stream.close();

	pointer = FromVF(vertex_shader_code, fragment_shader_code);
	loaded = true;
}

Shader::Shader(std::istream& stream, uint offset) : AssetObject(ASSETTYPE_SHADER) {
	string vertex_shader_code = "";
	string fragment_shader_code = "";
	//std::ifstream stream(p.c_str());
	stream.seekg(offset);
	if (!stream.good()) {
		std::cout << "shader not found!" << std::endl;
		return;
	}
	char* c = new char[4];
	stream.read(c, 3);
	c[3] = char0;
	string ss(c);
	if (string(c) != "KTS") {
		std::cerr << "file not supported" << std::endl;
		return;
	}

	int vs;
	_Strm2Val(stream, vs);

	char type;
	char* nmm = new char[100];
	for (int r = 0; r < vs; r++) {
		stream.get(type);
		byte bb = type;
		vars.push_back(new ShaderVariable());
		vars[r]->type = (SHADER_VARTYPE)bb;
		switch (bb) {
		case SHADER_INT:
			_Strm2Val(stream, vars[r]->min);
			_Strm2Val(stream, vars[r]->max);
			_Strm2Val(stream, vars[r]->val.i);
			break;
		case SHADER_FLOAT:
			_Strm2Val(stream, vars[r]->min);
			_Strm2Val(stream, vars[r]->max);
			_Strm2Val(stream, vars[r]->val.x);
			break;
		case SHADER_SAMPLER:
			byte bbb;
			_Strm2Val(stream, bbb);
			vars[r]->def.i = bbb;
			vars[r]->val.i = -1;
			break;
		}
		stream.getline(nmm, 100, char0);
		vars[r]->name += string(nmm);
	}
	stream.get(type);
	if ((byte)type != 0xff)
		return;

	int i;
	_Strm2Val(stream, i);
	char* cc = new char[i + 1];
	stream.read(cc, i);
	cc[i] = char0;
	vertex_shader_code = string(cc);
	delete[](cc);

	stream.get(type);
	if ((byte)type != 0x00)
		return;

	_Strm2Val(stream, i);
	cc = new char[i + 1];
	stream.read(cc, i);
	cc[i] = char0;
	fragment_shader_code = string(cc);
	delete[](cc);
	//stream.close();

	pointer = FromVF(vertex_shader_code, fragment_shader_code);
	loaded = true;
}

Shader::Shader(const string& vert, const string& frag) : AssetObject(ASSETTYPE_SHADER) {
	pointer = FromVF(vert, frag);
	loaded = true;
}

GLuint Shader::FromVF(const string& vert, const string& frag) {
	GLuint vertex_shader;
	string err = "";
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

GLuint Shader::FromF(GLuint vert, const string& frag) {
	GLuint vertex_shader = vert, fragment_shader;
	string err;

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
		Debug::Error("Shader", "Link error: " + string(&program_log[0]));
		glDeleteProgram(pointer);
		return 0;
	}

	glDetachShader(pointer, vertex_shader);
	glDetachShader(pointer, fragment_shader);
	glDeleteShader(fragment_shader);
	return pointer;
}