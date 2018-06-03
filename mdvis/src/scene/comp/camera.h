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
	rRenderTexture targetRT;
	std::vector<rCameraEffect> effects;

	void Render(RenderTexture* target = nullptr, renderFunc func = nullptr);

	uint GetIdAt(uint x, uint y);
	
	onBlitFunc onPreBlit, onBlit;
	GLuint d_fbo, d_colfbo, d_texs[4], d_idTex, d_depthTex, d_colTex;
	GLuint d_fbo2, d_texs2[4], d_depthTex2, _d_fbo, _d_texs[4], _d_depthTex;
	GLuint d_tfbo[NUM_EXTRA_TEXS], d_ttexs[NUM_EXTRA_TEXS];
	uint d_w, d_h, d_w2, d_h2, _d_w, _d_h, _w, _h;
	bool useGBuffer2, applyGBuffer2;
	void GenGBuffer2();
	void LoadGBuffer2(), UnloadGBuffer2();

	Camera(std::ifstream& stream, SceneObject* o, long pos = -1);

	std::vector<ASSETID> _effects;

	static void DrawSceneObjectsOpaque(std::vector<pSceneObject> oo, GLuint shader = 0);
	static void DrawSceneObjectsOverlay(std::vector<pSceneObject> oo, GLuint shader = 0);
	void RenderLights(GLuint targetFbo = 0);
	void DumpBuffers();

	//void _RenderProbesMask(std::vector<pSceneObject>& objs, Mat4x4 mat, std::vector<ReflectionProbe*>& probes), _RenderProbes(std::vector<ReflectionProbe*>& probes, Mat4x4 mat);
	//void _DoRenderProbeMask(ReflectionProbe* p, Mat4x4& ip), _DoRenderProbe(ReflectionProbe* p, Mat4x4& ip);
	static void _RenderSky(Mat4x4 ip, GLuint d_texs[], GLuint d_depthTex, float w = (float)Display::width, float h = (float)Display::height);
	void _DrawLights(std::vector<pSceneObject>& oo, Mat4x4& ip, GLuint targetFbo = 0);
	static void _ApplyEmission(GLuint d_fbo, GLuint d_texs[], float w = (float)Display::width, float h = (float)Display::height, GLuint targetFbo = 0);
	static void _DoDrawLight_Point(Light* l, Mat4x4& ip, GLuint d_fbo, GLuint d_texs[], GLuint d_depthTex, GLuint ctar, GLuint c_tex, float w = (float)Display::width, float h = (float)Display::height, GLuint targetFbo = 0);
	static void _DoDrawLight_Spot(Light* l, Mat4x4& ip, GLuint d_fbo, GLuint d_texs[], GLuint d_depthTex, GLuint ctar, GLuint c_tex, float w = (float)Display::width, float h = (float)Display::height, GLuint targetFbo = 0);
	static void _DoDrawLight_Spot_Contact(Light* l, Mat4x4& p, GLuint d_depthTex, float w, float h, GLuint src, GLuint tar);
	
	static void GenShaderFromPath(const string& pathv, const string& pathf, GLuint* program);
	static void GenShaderFromPath(GLuint vertex_shader, const string& path, GLuint* program);

	static GLuint rectIdBuf;
	static GLuint d_probeMaskProgram, d_probeProgram, d_blurProgram, d_blurSBProgram, d_skyProgram, d_pLightProgram, d_sLightProgram, d_sLightCSProgram, d_sLightRSMProgram, d_sLightRSMFluxProgram;
	static GLuint d_reflQuadProgram;
	static GLint d_skyProgramLocs[];

	static GLuint emptyVao;
	static const float fullscreenVerts[];
	static const int fullscreenIndices[];

	int _tarRT;
	float quality, quality2;

	static std::unordered_map<string, GLuint> fetchTextures;
	static std::vector<string> fetchTexturesUpdated;
	static GLuint DoFetchTexture(string s);
	static void ClearFetchTextures();

	static const string _gbufferNames[];

	void ApplyGL();

	static void InitShaders();
	void InitGBuffer(uint w, uint h);
};
