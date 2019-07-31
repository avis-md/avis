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

#pragma once
#include "Engine.h"

enum GBUFFER : GLenum {
	GBUFFER_COLOR = GL_COLOR_ATTACHMENT0,
	GBUFFER_ID,
	GBUFFER_NORMAL
};
const byte NUM_EXTRA_TEXS = 3;

enum class RENDER_PASS : byte {
	SOLID,
	TRANS
};

typedef void(*onBlitFunc)();

class Camera : public Component {
public:
	Camera();

	static Camera* active;

	bool ortographic;
	float fov, orthoSize;
	Rect screenPos;
	CAM_CLEARTYPE clearType;
	Vec4 clearColor;
	float nearClip;
	float farClip;
	GLuint target;

	float scale;
	Vec2 offset;

	void Render(onBlitFunc func = nullptr);

	uint GetIdAt(uint x, uint y);
	
	onBlitFunc onPreBlit, onBlit;
	struct TexGroup {
		GLuint fbo, colTex, idTex, normTex, depthTex;

		TexGroup() : fbo(0), colTex(0), idTex(0), normTex(0), depthTex(0) {}
		void Clear();
	} texs = {}, texs2 = {}, _texs = {}, trTexs = {};
	
	GLuint blitFbos[NUM_EXTRA_TEXS], blitTexs[NUM_EXTRA_TEXS];
	void SwapBlitBuffers();

	uint d_w, d_h, d_w2, d_h2, _d_w, _d_h, _w, _h;
	bool useGBuffer2, applyGBuffer2;
	void GenGBuffer2();

	Camera(std::ifstream& stream, SceneObject* o, long pos = -1);

	static GLuint rectIdBuf;
	//static GLuint d_probeMaskProgram, d_probeProgram, d_blurProgram, d_blurSBProgram, d_skyProgram, d_pLightProgram, d_sLightProgram, d_sLightCSProgram, d_sLightRSMProgram, d_sLightRSMFluxProgram;
	//static GLuint d_reflQuadProgram;
	//static GLint d_skyProgramLocs[];

	static GLuint emptyVao;
	static const float fullscreenVerts[];
	static const int fullscreenIndices[];

	int _tarRT;
	float quality, quality2;

	void ApplyGL();

	static void InitShaders();
	void InitGBuffer(uint w, uint h);
};
