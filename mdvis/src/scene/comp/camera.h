#pragma once
#include "Engine.h"

const byte NUM_EXTRA_TEXS = 3;

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
	rRenderTexture targetRT; //oh well
	std::vector<rCameraEffect> effects;

	void Render(RenderTexture* target = nullptr, onBlitFunc func = nullptr);

	uint GetIdAt(uint x, uint y);
	
	onBlitFunc onPreBlit, onBlit;
	GLuint d_fbo, d_colfbo, d_texs[4], d_idTex, d_depthTex, d_colTex;
	GLuint d_fbo2, d_texs2[4], d_depthTex2, _d_fbo, _d_texs[4], _d_depthTex;
	GLuint d_tfbo[NUM_EXTRA_TEXS], d_ttexs[NUM_EXTRA_TEXS];
	uint d_w, d_h, d_w2, d_h2, _d_w, _d_h, _w, _h;
	bool useGBuffer2, applyGBuffer2;
	void GenGBuffer2();

	Camera(std::ifstream& stream, SceneObject* o, long pos = -1);

	std::vector<ASSETID> _effects;

	static void GenShaderFromPath(const char* vs, const char* fs, GLuint* program);
	static void GenShaderFromPath(GLuint vertex_shader, const char* fs, GLuint* program);

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
