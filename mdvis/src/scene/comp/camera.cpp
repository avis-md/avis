#include "Engine.h"
#include "res/shddata.h"

Camera* Camera::active;

Camera::Camera() : Component("Camera", COMP_CAM), 
	ortographic(false), fov(60), orthoSize(10), screenPos(0.f, 0.f, 1.f, 1.f), 
	clearType(CAM_CLEAR_COLOR), clearColor(black(1)), _tarRT(-1), 
	nearClip(0.01f), farClip(500), quality(1), quality2(0.5f), 
	useGBuffer2(false), applyGBuffer2(false), 
	target(0), scale(1), offset() {
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
	Quat q = glm::inverse(object->transform.rotation());
	if (ortographic) {
		float hw = Display::height * 1.f / Display::width;
		MVP::Mul(glm::ortho(-1.f, 1.f, -hw, hw, 0.01f, 500.f));
	} else {
		MVP::Mul(glm::perspectiveFov(fov * deg2rad, static_cast<float>(Display::width), static_cast<float>(Display::height), 0.01f, 500.f));
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
	glGenVertexArrays(1, &emptyVao);
	
	glGenBuffers(1, &rectIdBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectIdBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint), fullscreenIndices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}