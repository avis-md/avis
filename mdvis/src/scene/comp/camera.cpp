#include "Engine.h"
#include "Editor.h"

const int Camera::camVertsIds[19] = { 0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 4, 4, 3, 3, 1, 1, 2, 5 };

Camera::Camera() : Component("Camera", COMP_CAM, DRAWORDER_NONE), ortographic(false), fov(60), orthoSize(10), screenPos(0.0f, 0.0f, 1.0f, 1.0f), clearType(CAM_CLEAR_COLOR), clearColor(black(1)), _tarRT(-1), nearClip(0.01f), farClip(500) {
#ifdef IS_EDITOR
	UpdateCamVerts();
#else
	//if (!d_fbo)
	InitGBuffer();
#endif
}

Camera::Camera(std::ifstream& stream, SceneObject* o, long pos) : Camera() {
	if (pos >= 0)
		stream.seekg(pos);
	_Strm2Val(stream, fov);
	_Strm2Val(stream, screenPos.x);
	_Strm2Val(stream, screenPos.y);
	_Strm2Val(stream, screenPos.w);
	_Strm2Val(stream, screenPos.h);

	/*
	#ifndef IS_EDITOR
	if (d_skyProgram == 0) {
	InitShaders();
	}
	#endif
	*/
}

void Camera::ApplyGL() {
	switch (clearType) {
	case CAM_CLEAR_COLOR:
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearDepthf(1);
		glClear(GL_DEPTH_BUFFER_BIT);
		break;
	case CAM_CLEAR_DEPTH:
		glClearDepthf(1);
		glClear(GL_DEPTH_BUFFER_BIT);
		break;
	}

	MVP::Switch(true);
	MVP::Clear();
	Quat q = glm::inverse(object->transform.rotation());
	MVP::Mul(glm::perspectiveFov(fov * deg2rad, (float)Display::width, (float)Display::height, 0.01f, 500.0f));
	MVP::Scale(1, 1, -1);
	MVP::Mul(QuatFunc::ToMatrix(q));
	Vec3 pos = -object->transform.position();
	MVP::Translate(pos.x, pos.y, pos.z);
}

void Camera::GenShaderFromPath(const string& pathv, const string& pathf, GLuint* program) {
	GLuint vertex_shader;
	std::string err;
	if (!Shader::LoadShader(GL_VERTEX_SHADER, DefaultResources::GetStr(pathv), vertex_shader, &err)) {
		Debug::Error("Cam Shader Compiler", pathv + "! " + err);
		abort();
	}
	GenShaderFromPath(vertex_shader, pathf, program);
}

void Camera::GenShaderFromPath(GLuint vertex_shader, const string& path, GLuint* program) {
	GLuint fragment_shader;
	std::string err;
	if (!Shader::LoadShader(GL_FRAGMENT_SHADER, DefaultResources::GetStr(path), fragment_shader, &err)) {
		Debug::Error("Cam Shader Compiler", path + "! " + err);
		abort();
	}

	*program = glCreateProgram();
	glAttachShader(*program, vertex_shader);
	glAttachShader(*program, fragment_shader);
	glLinkProgram(*program);
	GLint link_result;
	glGetProgramiv(*program, GL_LINK_STATUS, &link_result);
	if (link_result == GL_FALSE)
	{
		int info_log_length = 0;
		glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(*program, info_log_length, NULL, &program_log[0]);
		std::cout << "cam shader error" << std::endl << &program_log[0] << std::endl;
		glDeleteProgram(*program);
		*program = 0;
		abort();
	}
	glDetachShader(*program, vertex_shader);
	glDetachShader(*program, fragment_shader);
	glDeleteShader(fragment_shader);
}

void Camera::InitShaders() {
	int link_result = 0;
	GLuint vertex_shader;
	string err = "";

	if (!Shader::LoadShader(GL_VERTEX_SHADER, DefaultResources::GetStr("lightPassVert.txt"), vertex_shader, &err)) {
		Debug::Error("Cam Shader Compiler", "v! " + err);
		abort();
	}
	GenShaderFromPath(vertex_shader, "blurPassFrag.txt", &d_blurProgram);
	GenShaderFromPath(vertex_shader, "blurPassFrag_Skybox.txt", &d_blurSBProgram);
	GenShaderFromPath(vertex_shader, "lightPassFrag_Sky.txt", &d_skyProgram);
	GenShaderFromPath(vertex_shader, "lightPassFrag_Point.txt", &d_pLightProgram);
	GenShaderFromPath(vertex_shader, "lightPassFrag_Spot.txt", &d_sLightProgram);
	GenShaderFromPath(vertex_shader, "lightPassFrag_Spot_ContShad.txt", &d_sLightCSProgram);
	GenShaderFromPath(vertex_shader, "lightPassFrag_Spot_GI_RSM.txt", &d_sLightRSMProgram);
	GenShaderFromPath(vertex_shader, "lightPassFrag_Spot_GI_FluxPrep.txt", &d_sLightRSMFluxProgram);
	GenShaderFromPath(vertex_shader, "lightPassFrag_ProbeMask.txt", &d_probeMaskProgram);
	GenShaderFromPath(vertex_shader, "lightPassFrag_ReflQuad.txt", &d_reflQuadProgram);
	glDeleteShader(vertex_shader);

	d_skyProgramLocs[0] = glGetUniformLocation(d_skyProgram, "_IP");
	d_skyProgramLocs[1] = glGetUniformLocation(d_skyProgram, "inColor");
	d_skyProgramLocs[2] = glGetUniformLocation(d_skyProgram, "inNormal");
	d_skyProgramLocs[3] = glGetUniformLocation(d_skyProgram, "inSpec");
	d_skyProgramLocs[4] = glGetUniformLocation(d_skyProgram, "inDepth");
	d_skyProgramLocs[5] = glGetUniformLocation(d_skyProgram, "inSky");
	d_skyProgramLocs[6] = glGetUniformLocation(d_skyProgram, "skyStrength");
	d_skyProgramLocs[7] = glGetUniformLocation(d_skyProgram, "screenSize");
	d_skyProgramLocs[8] = glGetUniformLocation(d_skyProgram, "skyStrengthB");

	glGenBuffers(1, &fullscreenVbo);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenVbo);
	glBufferStorage(GL_ARRAY_BUFFER, 4 * sizeof(Vec2), &fullscreenVerts[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenVertexArrays(1, &fullscreenVao);
	glBindVertexArray(fullscreenVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenVbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Light::ScanParams();
}

void Camera::UpdateCamVerts() {}