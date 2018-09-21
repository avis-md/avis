#include "Engine.h"
#include "res/shddata.h"

Camera* Camera::active;

Camera::Camera() : Component("Camera", COMP_CAM), ortographic(false), fov(60), orthoSize(10), screenPos(0.0f, 0.0f, 1.0f, 1.0f), clearType(CAM_CLEAR_COLOR), clearColor(black(1)), _tarRT(-1), nearClip(0.01f), farClip(500), quality(1), quality2(0.5f), useGBuffer2(false), applyGBuffer2(false), target(0) {
	InitGBuffer(Display::width, Display::height);
}

Camera::Camera(std::ifstream& stream, SceneObject* o, long pos) : Camera() {
	if (pos >= 0)
		stream.seekg(pos);
	_Strm2Val(stream, fov);
	_Strm2Val(stream, screenPos.x);
	_Strm2Val(stream, screenPos.y);
	_Strm2Val(stream, screenPos.w);
	_Strm2Val(stream, screenPos.h);
}

void Camera::ApplyGL() {
	MVP::Switch(true);
	MVP::Clear();
	Quat q = glm::inverse(object->transform.rotation());
	if (ortographic) {
		float hw = Display::height * 1.0f / Display::width;
		MVP::Mul(glm::ortho(-1.0f, 1.0f, -hw, hw, 0.01f, 500.0f));
	} else {
		MVP::Mul(glm::perspectiveFov(fov * deg2rad, (float)Display::width, (float)Display::height, 0.01f, 500.0f));
	}
	MVP::Scale(1, 1, -1);
	MVP::Mul(QuatFunc::ToMatrix(q));
	Vec3 pos = -object->transform.position();
	MVP::Translate(pos.x, pos.y, pos.z);
}

void Camera::InitShaders() {
	int link_result = 0;
	GLuint vertex_shader;
	std::string err = "";

	if (!Shader::LoadShader(GL_VERTEX_SHADER, glsl::minVert, vertex_shader, &err)) {
		Debug::Error("Cam Shader Compiler", "v! " + err);
		abort();
	}
	/*
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
	*/
	glGenVertexArrays(1, &emptyVao);
	
	glGenBuffers(1, &rectIdBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectIdBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint), fullscreenIndices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}