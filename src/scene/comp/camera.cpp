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

#include "Engine.h"
#include "res/shd/minVert.h"

Camera* Camera::active;

Camera::Camera() : Component("Camera", COMP_CAM), 
	ortographic(false), fov(60), orthoSize(10), screenPos(0.f, 0.f, 1.f, 1.f),
	clearType(CAM_CLEAR_COLOR), clearColor(black(1)), nearClip(0.01f), farClip(500),
	target(0), scale(1), offset(), 
	useGBuffer2(false), applyGBuffer2(false), _tarRT(-1), quality(1), quality2(0.5f) {
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
	if (ortographic) {
		float hw = Display::height * 1.f / Display::width;
		MVP::Mul(glm::ortho(-1.f, 1.f, -hw, hw, 0.01f, 500.f));
	}
	else {
		MVP::Mul(glm::perspectiveFov(fov * deg2rad, static_cast<float>(Display::width), static_cast<float>(Display::height), 0.01f, 500.f));
	}
	MVP::Push();
	Quat q = glm::inverse(object->transform.rotation());
	MVP::Scale(1, 1, -1);
	MVP::Mul(QuatFunc::ToMatrix(q));
	Vec3 pos = -object->transform.position();
	MVP::Translate(pos.x, pos.y, pos.z);
}

void Camera::InitShaders() {
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