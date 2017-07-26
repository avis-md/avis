#include "Engine.h"
#include "Editor.h"
#include <GL/glew.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>

bool ShaderBase::LoadShader(GLenum shaderType, string source, GLuint& shader, string* err) {

	int compile_result = 0;

	shader = glCreateShader(shaderType);
	const char *shader_code_ptr = source.c_str();
	const int shader_code_size = source.size();

	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);

	//check for errors
	if (compile_result == GL_FALSE)
	{
		int info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> shader_log(info_log_length);
		glGetShaderInfoLog(shader, info_log_length, NULL, &shader_log[0]);
		glDeleteShader(shader);
		shader = 0;
		if (err != nullptr)
			*err += string(shader_log.begin(), shader_log.end());
		return false;
	}
	//std::std::cout << "shader compiled" << std::endl;
	return true;
}

ShaderBase::ShaderBase(string path) : AssetObject(ASSETTYPE_SHADER) {
	string p = Editor::instance->projectFolder + "Assets\\" + path + ".meta";
	string vertex_shader_code = "";
	string fragment_shader_code = "";
	std::ifstream stream(p.c_str());
	if (!stream.good()) {
		std::cout << "shader not found!" << std::endl;
		return;
	}
	char* c = new char[4];
	stream.read(c, 3);
	c[3] = (char)0;
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
		vars[r]->type = bb;
		switch (bb) {
		case SHADER_INT:
			_Strm2Val(stream, vars[r]->min);
			_Strm2Val(stream, vars[r]->max);
			_Strm2Val(stream, vars[r]->val.i);
			stream.getline(nmm, 100, (char)0);
			vars[r]->name += string(nmm);
			break;
		case SHADER_FLOAT:
			_Strm2Val(stream, vars[r]->min);
			_Strm2Val(stream, vars[r]->max);
			_Strm2Val(stream, vars[r]->val.x);
			stream.getline(nmm, 100, (char)0);
			vars[r]->name += string(nmm);
			break;
		case SHADER_SAMPLER:
			vars[r]->val.i = -1;
			stream.getline(nmm, 100, (char)0);
			vars[r]->name += string(nmm);
			break;
		}
	}
	stream.get(type);
	if ((byte)type != 0xff)
		return;

	int i;
	_Strm2Val(stream, i);
	char* cc = new char[i+1];
	stream.read(cc, i);
	cc[i] = (char)0;
	vertex_shader_code = string(cc);
	delete[](cc);

	stream.get(type);
	if ((byte)type != 0x00)
		return;

	_Strm2Val(stream, i);
	cc = new char[i + 1];
	stream.read(cc, i);
	cc[i] = (char)0;
	fragment_shader_code = string(cc);
	delete[](cc);
	stream.close();

	GLuint vertex_shader, fragment_shader;
	string err = "";
	if (vertex_shader_code != "") {
		std::cout << "Vertex Shader: " << std::endl << vertex_shader_code;
		if (!LoadShader(GL_VERTEX_SHADER, vertex_shader_code, vertex_shader, &err)) {
			Editor::instance->_Error("Shader Compiler", path + " " + err);
			return;
		}
	}
	else return;
	if (fragment_shader_code != "") {
		std::cout << "Fragment Shader: " << std::endl << fragment_shader_code;
		if (!LoadShader(GL_FRAGMENT_SHADER, fragment_shader_code, fragment_shader, &err)) {
			Editor::instance->_Error("Shader Compiler", path + " " + err);
			return;
		}
	}
	else return;

	pointer = glCreateProgram();
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
		std::cout << "Shader link error" << std::endl << &program_log[0] << std::endl;
		glDeleteProgram(pointer);
		pointer = 0;
		return;
	}
	std::cout << "shader linked" << std::endl;

	glDetachShader(pointer, vertex_shader);
	glDeleteShader(vertex_shader);
	glDetachShader(pointer, fragment_shader);
	glDeleteShader(fragment_shader);
	loaded = true;
}

ShaderBase::ShaderBase(std::ifstream& stream, uint offset) : AssetObject(ASSETTYPE_SHADER) {
	if (!stream.is_open())
		return;
	stream.seekg(offset);
	string vertex_shader_code = "";
	string fragment_shader_code = "";
	char* c = new char[4];
	stream.read(c, 3);
	c[3] = (char)0;
	string ss(c);
	if (string(c) != "KTS") {
		Debug::Error("ShaderLoader", "Wrong data header!");
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
		vars[r]->type = bb;
		switch (bb) {
		case SHADER_INT:
			_Strm2Val(stream, vars[r]->min);
			_Strm2Val(stream, vars[r]->max);
			_Strm2Val(stream, vars[r]->val.i);
			stream.getline(nmm, 100, (char)0);
			vars[r]->name += string(nmm);
			break;
		case SHADER_FLOAT:
			_Strm2Val(stream, vars[r]->min);
			_Strm2Val(stream, vars[r]->max);
			_Strm2Val(stream, vars[r]->val.x);
			stream.getline(nmm, 100, (char)0);
			vars[r]->name += string(nmm);
			break;
		case SHADER_SAMPLER:
			vars[r]->val.i = -1;
			stream.getline(nmm, 100, (char)0);
			vars[r]->name += string(nmm);
			break;
		}
	}
	stream.get(type);
	if ((byte)type != 0xff)
		return;

	int i;
	_Strm2Val(stream, i);
	char* cc = new char[i + 1];
	stream.read(cc, i);
	cc[i] = (char)0;
	vertex_shader_code = string(cc);
	delete[](cc);

	stream.get(type);
	if ((byte)type != 0x00)
		return;

	_Strm2Val(stream, i);
	cc = new char[i + 1];
	stream.read(cc, i);
	cc[i] = (char)0;
	fragment_shader_code = string(cc);
	delete[](cc);

	GLuint vertex_shader, fragment_shader;
	string err = "";
	if (vertex_shader_code != "") {
		std::cout << "Vertex Shader: " << std::endl << vertex_shader_code;
		if (!LoadShader(GL_VERTEX_SHADER, vertex_shader_code, vertex_shader, &err)) {
			Debug::Error("Shader Compiler", "Vert error: " + err);
			return;
		}
	}
	else return;
	if (fragment_shader_code != "") {
		std::cout << "Fragment Shader: " << std::endl << fragment_shader_code;
		if (!LoadShader(GL_FRAGMENT_SHADER, fragment_shader_code, fragment_shader, &err)) {
			Debug::Error("Shader Compiler", "Frag error: " + err);
			return;
		}
	}
	else return;

	pointer = glCreateProgram();
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
		std::cout << "Shader link error" << std::endl << &program_log[0] << std::endl;
		glDeleteProgram(pointer);
		pointer = 0;
		return;
	}
	std::cout << "shader linked" << std::endl;

	glDetachShader(pointer, vertex_shader);
	glDeleteShader(vertex_shader);
	glDetachShader(pointer, fragment_shader);
	glDeleteShader(fragment_shader);
	loaded = true;
}

/*
ShaderBase::ShaderBase(string vertex_shader_code, string fragment_shader_code) : AssetObject(ASSETTYPE_SHADER) {
	GLuint vertex_shader, fragment_shader;
	if (vertex_shader_code != "") {
		std::cout << "Vertex Shader: " << std::endl << vertex_shader_code;
		if (!LoadShader(GL_VERTEX_SHADER, vertex_shader_code, vertex_shader))
			return;
	}
	else return;
	if (fragment_shader_code != "") {
		std::cout << "Fragment Shader: " << std::endl << fragment_shader_code;
		if (!LoadShader(GL_FRAGMENT_SHADER, fragment_shader_code, fragment_shader))
			return;
	}
	else return;

	pointer = glCreateProgram();
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
		std::cout << "Shader link error" << std::endl << &program_log[0] << std::endl;
		glDeleteProgram(pointer);
		pointer = 0;
		return;
	}
	std::cout << "shader linked" << std::endl;

	glDetachShader(pointer, vertex_shader);
	glDeleteShader(vertex_shader);
	glDetachShader(pointer, fragment_shader);
	glDeleteShader(fragment_shader);
	loaded = true;
}

/* shader meta format, (x) = x bytes
SHD
//params
[type(1)]{[min(4)][max(4)][default(4)](optional)}[name]\0

VARSTART
int range(0, 100) foo = 1;
float range(-20.0, 200.0) boo = 2.5;
texture mytex;
VAREND

//code
VRT
[size(4)][codestring]\0
FRG
[size(4)][codestring]\0
*/
bool ShaderBase::Parse(std::ifstream* stream, string path) {
	string a, text;
	std::vector<string> included;
	byte readingType = 0;

	//resolve includes
	while (!(*stream).eof()) {
		getline(*stream, a);
		if (a.size() > 9 && a.substr(0, 9) == "#include ") {
			string nmm = "", nm = a.substr(9, string::npos);
			for (uint r = 0; r < nm.size(); r++) {
				char c = nm[r];
				if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
					nmm += c;
			}
			if (find(included.begin(), included.end(), a) >= included.begin() + included.size()) {
				included.push_back(a);
				string path(Editor::dataPath + "ShaderIncludes\\" + nmm + ".shadinc");
				string in = IO::ReadFile(path);
				if (in != "") {
					std::cout << nmm << ".shadinc included" << std::endl;
					//text += "//Included from " + nmm + ".shadinc\n";
					text += in + "\n";
					//text += "//end of " + nmm + ".shadinc\n";
				}
				else {
					//text += "//" + a + " (not found!)\n";
				}
			}
		}
		else text += a + "\n";
	}

	string t2(text);
	text = "";
	bool wasn = false;
	for (char c : t2) {
		if (c != '\r' && c != '\t' && !(wasn && (c == ' ' || c == '\n'))) {
			text += c;
			wasn = (c == '\n');
		}
	}
	text += char(0);

	std::stringstream sstream;
	sstream.str(text);

	//VAR (vars), IN (in @ vert), V2F (out @ vert, in @ frag), COMMON (copy to both), VERT, FRAG
	string in, v2f, common, vert, frag;
	std::vector<ShaderVariable*> vrs;
	int vrSize = -1;
	string vertCode, fragCode;

	while (!sstream.eof()) {
		//std::cout << to_string(readingType) << std::endl;
		if (readingType == 0) {
			getline(sstream, a);
			if (a == "VARSTART")
				readingType = 1;
			else if (a == "INSTART")
				readingType = 2;
			else if (a == "V2FSTART")
				readingType = 3;
			else if (a == "COMMONSTART")
				readingType = 4;
			else if (a == "VERTSTART")
				readingType = 5;
			else if (a == "FRAGSTART")
				readingType = 6;
			//else if (a != ""){
				//Editor::instance->_Warning("Shader Importer", "Unscoped code found in shader. They will be ignored.");
				//std::cout << ">" << a << std::endl;
			//}
		}
		else if (readingType == 1) {
			string x;
			sstream >> x;
			if (x == "VAREND")
				readingType = 0;
			else {
				/*VARSTART
				int range(0, 100) foo = 1;
				float range(-20.0, 200.0) boo = 2.5;
				texture mytex = white;
				*/
				bool nr = false; //range disallowed?
				byte vt = 0; //i, f
				vrs.push_back(new ShaderVariable());
				vrSize++;
				vrs[vrSize]->min = std::numeric_limits<float>::lowest();
				vrs[vrSize]->max = std::numeric_limits<float>::infinity();
				vrs[vrSize]->val = ShaderValue();
				vrs[vrSize]->def = ShaderValue();
				if (x == "int") {
					vrs[vrSize]->type = SHADER_INT;
				}
				else if (x == "float") {
					vrs[vrSize]->type = SHADER_FLOAT;
					vt = 1;
				}
				else if (x == "texture") {
					vrs[vrSize]->type = SHADER_SAMPLER;
					vrs[vrSize]->val.i = -1;
					nr = true;
				}
				else {
					Editor::instance->_Error("Shader Importer", "Unknown variable string (" + x + ")!");
					return false;
				}

				sstream >> x;
				if (x.find("range(") == 0) {
					if (nr) {
						Editor::instance->_Error("Shader Importer", "Range not allowed on this variable!");
						return false;
					}
					if (x.size() > 6) {
						x = x.substr(6, string::npos);
					}
					else
						sstream >> x;
					int c = x.find(',');
					if (c != string::npos) {
						try {
							vrs[vrSize]->min = stof(x.substr(0, c));
							string r;
							while (x.find(')') == string::npos) {
								sstream >> r;
								x += r;
							}
							vrs[vrSize]->max = stof(x.substr(c + 1, x.find(')') - c - 1));
							if (x.find(')') != x.size() - 1) {
								x = x.substr(x.find(')') + 1);
							}
							else
								sstream >> x;
						}
						catch (std::exception e) {
							Editor::instance->_Error("Shader Importer", "Range syntax error in shader!");
							return false;
						}
					}
				}
				string r;
				while (x.find(';') == string::npos) {
					sstream >> r;
					if (sstream.eof()) {
						Editor::instance->_Error("Shader Importer", "Unexpected EOF in variables!");
						return false;
					}
					x += r;
				}
				int y = x.find('=');
				if (y != string::npos) {
					vrs[vrSize]->name = x.substr(0, y);
					string def = x.substr(y + 1, x.find(';') - y - 1);
					try {
						if (vrs[vrSize]->type == SHADER_INT)
							vrs[vrSize]->def.i = stoi(def);
						else if (vrs[vrSize]->type == SHADER_FLOAT)
							vrs[vrSize]->def.x = stof(def);
						else if (vrs[vrSize]->type == SHADER_SAMPLER) {
							if (def == "black")
								vrs[vrSize]->def.i = Material::defTex_Black;
							else if (def == "grey")
								vrs[vrSize]->def.i = Material::defTex_Grey;
							else if (def == "white")
								vrs[vrSize]->def.i = Material::defTex_White;
							else
								Editor::instance->_Error("Shader Importer", "Default variable value (" + def + ") cannot be parsed!");
							return false;
						}
					}
					catch (...) {
						Editor::instance->_Error("Shader Importer", "Default variable value (" + def + ") cannot be parsed!");
						return false;
					}
				}
				else {
					vrs[vrSize]->name = x.substr(0, x.find(';'));
					if (vrs[vrSize]->type == SHADER_SAMPLER) { //sampler default is black
						vrs[vrSize]->def.i = Material::defTex_Black;
					}
				}
			}
		}
		else if (readingType == 2) {
			getline(sstream, a);
			if (a == "INEND")
				readingType = 0;
			else {
				if (a.substr(0, 6) == "layout" && a.size() > 6 && a[6] != '(') {
					try {
						int rr = stoi(a.substr(6, 1));
						string ss = a.substr(6, string::npos);
						int r = stoi(ss, nullptr);
						in += "layout(position=" + to_string(r) + ")" + ss + "\n";
					}
					catch (std::exception e) {
						in += a + "\n";
					}
				}
				else
					in += a + "\n";
			}
		}
		else {
			getline(sstream, a);
			if (readingType == 3) {
				if (a == "V2FEND")
					readingType = 0;
				else
					v2f += a + "\n";
			}
			else if (readingType == 4) {
				if (a == "COMMONEND")
					readingType = 0;
				else
					common += a + "\n";
			}
			else if (readingType == 5) {
				if (a == "VERTEND")
					readingType = 0;
				else
					vert += a + "\n";
			}
			else if (readingType == 6) {
				if (a == "FRAGEND")
					readingType = 0;
				else
					frag += a + "\n";
			}
		}
	}

	//combine everything
	vertCode = "#version 330 core\n" + in + "\n\n" + common + "\n\n";
	fragCode = "#version 330 core\n" + common + "\n\n";
	std::stringstream v2fStream;
	v2fStream.str(v2f);
	while (!v2fStream.eof()) {
		getline(v2fStream, a);
		if (a == "")
			continue;
		vertCode += "out " + a + "\n";
		fragCode += "in " + a + "\n";
	}
	vertCode += vert;
	fragCode += frag;

	if (IO::HasFile(path.c_str())) {
		remove(path.c_str());
	}
	std::ofstream strm;
	strm.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!strm.is_open()) {
		Editor::instance->_Error("Shader Importer", "Cannot write to " + path);
		return false;
	}
	strm << "KTS";
	int ii = vrs.size();
	_StreamWrite(&ii, &strm, 4);
	for (ShaderVariable* sv : vrs) {
		strm << sv->type;
		if (sv->type == SHADER_FLOAT || sv->type == SHADER_INT) {
			_StreamWrite(&sv->min, &strm, 4);
			_StreamWrite(&sv->max, &strm, 4);
			if (sv->type == SHADER_FLOAT)
				_StreamWrite(&sv->val.x, &strm, 4);
			else
				_StreamWrite(&sv->val.i, &strm, 4);
		}
		strm << sv->name << (char)0;
	}
	strm << (char)255;
	int s = vertCode.size();
	_StreamWrite(&s, &strm, 4);
	strm << vertCode << (char)0;
	s = fragCode.size();
	_StreamWrite(&s, &strm, 4);
	strm << fragCode << (char)0;

	strm.close();
	return true;
}

/*old shader class

GLuint Shader::pointer = 0;

GLuint Shader::LoadShader(GLenum shaderType, string source) {

	int compile_result = 0;

	GLuint shader = glCreateShader(shaderType);
	const char *shader_code_ptr = source.c_str();
	const int shader_code_size = source.size();

	glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);

	//check for errors
	if (compile_result == GL_FALSE)
	{

		int info_log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> shader_log(info_log_length);
		glGetShaderInfoLog(shader, info_log_length, NULL, &shader_log[0]);
		std::cerr << "error compiling shader" << std::endl;
		return 0;
	}
	std::std::cout << "shader compiled" << std::endl;
	return shader;
}

GLuint Shader::CreateProgram(string& path){
	string vertex_shader_code = "";
	string fragment_shader_code = "";
	std::ifstream stream(path.c_str());
	if (!stream.good()) {
		std::cout << "not found!" << std::endl;
		return 0;
	}
	string a;
	bool hasData;
	int x;
	stream >> a;
	if (a != "KTS123") {
		std::cerr << "file not supported" << std::endl;
		return 0;
	}
	int readingType = 0;
	while (!stream.eof()) {
		getline(stream, a);
		if (readingType == 0) {
			if (a == "VERTEXBEGIN") {
				readingType = 1;
			}
			else if (a == "FRAGMENTBEGIN") {
				readingType = 2;
			}
		}
		else if (readingType == 1) {
			if (a == "VERTEXEND") {
				readingType = 0;
			}
			else if (a != ""){
				vertex_shader_code += a + "\n";
				hasData = true;
			}
		}
		else if (readingType == 2) {
			if (a == "FRAGMENTEND") {
				readingType = 0;
			}
			else if (a != ""){
				fragment_shader_code += a + "\n";
				hasData = true;
			}
		}
	}

	GLuint program = glCreateProgram();
	GLuint vertex_shader, fragment_shader;
	if (vertex_shader_code != "") {
		std::cout << "Vertex Shader: " << std::endl << vertex_shader_code;
		vertex_shader = LoadShader(GL_VERTEX_SHADER, vertex_shader_code);
		glAttachShader(program, vertex_shader);
	}
	else return 0;
	if (fragment_shader_code != "") {
		std::cout << "Fragment Shader: " << std::endl << fragment_shader_code;
		fragment_shader = LoadShader(GL_FRAGMENT_SHADER, fragment_shader_code);
		glAttachShader(program, fragment_shader);
	}
	else return 0;

	int link_result = 0;

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_result);
	if (link_result == GL_FALSE)
	{

		int info_log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(program, info_log_length, NULL, &program_log[0]);
		std::cout << "Shader link error" << std::endl << &program_log[0] << std::endl;
		return 0;
	}
	std::cout << "shader linked" << std::endl;

	glDetachShader(program, vertex_shader);
	glDeleteShader(vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(fragment_shader);
	
	pointer = program;
	return program;
}


GLuint Shader::GetTexture(const string& path) {
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize;   // = width*height*3
	unsigned char *data;

	FILE *file;
	fopen_s(&file, path.c_str(), "rb");
	if (!file){ printf("Image could not be opened\n"); return 0; }
	if (fread(header, 1, 54, file) != 54){ // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return false;
	}
	if (header[0] != 'B' || header[1] != 'M'){
		printf("Not a correct BMP file\n");
		return 0;
	}
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way
	data = new unsigned char[imageSize];
	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);
	//Everything is in memory now, the file can be closed
	fclose(file);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}
/*

bool Shader::SetTexture(GLuint program, GLchar* name, string& path, int w, int h, GLenum slot) {
	std::ifstream stream(path.c_str());
	if (!stream.good()) {
		std::cout << "not found!" << std::endl;
		return false;
	}
	unsigned char* tex = GetTexture(path, w, h);
	GLuint texi;
	glGenTextures(1, &texi);
	glUseProgram(program);
	glActiveTexture(slot);
	glBindTexture(GL_TEXTURE_2D, texi);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	GLint loc = glGetUniformLocation(program, name);
	if (loc != -1)
	{
		glUniform1i(loc, 0);
		glUseProgram(0);
		return true;
	}
	else return false;
}

void Shader::SetUniform(GLuint program, GLchar* name, float val) {
	glUseProgram(program);
	GLint loc = glGetUniformLocation(program, name);
	if (loc != -1)
	{
		glUniform1f(loc, val);
	}
	glUseProgram(0);
}

void Shader::SetWindow(GLuint program, float w, float h) {
	glUseProgram(program);
	GLint loc = glGetUniformLocation(program, "_windowSize");
	if (loc != -1)
	{
		glUniform2f(loc, w, h);
	}
	else
		std::cout << "program uniform not found";
	glUseProgram(0);
}
*/