#include "Engine.h"
#include <random>

void _InitGBuffer(Camera::TexGroup& tg, float w = Display::width, float h = Display::height) {
	glGenFramebuffers(1, &tg.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tg.fbo);

	glGenTextures(1, &tg.colTex);
	glGenTextures(1, &tg.idTex);
	glGenTextures(1, &tg.normTex);
	glGenTextures(1, &tg.depthTex);

	// color
	glBindTexture(GL_TEXTURE_2D, tg.colTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)w, (int)h, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GBUFFER_COLOR, GL_TEXTURE_2D, tg.colTex, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	//id
	glBindTexture(GL_TEXTURE_2D, tg.idTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32UI, (int)w, (int)h, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GBUFFER_ID, GL_TEXTURE_2D, tg.idTex, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	//normal
	glBindTexture(GL_TEXTURE_2D, tg.normTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)w, (int)h, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GBUFFER_NORMAL, GL_TEXTURE_2D, tg.normTex, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	// depth
	glBindTexture(GL_TEXTURE_2D, tg.depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, (int)w, (int)h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tg.depthTex, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		Debug::Error("Camera", "FB error:" + std::to_string(status));
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Camera::TexGroup::Clear() {
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colTex);
	glDeleteTextures(1, &idTex);
	glDeleteTextures(1, &normTex);
	glDeleteTextures(1, &depthTex);
}

void Camera::GenGBuffer2() {
	useGBuffer2 = true;
	uint dw2 = uint(d_w * quality2);
	uint dh2 = uint(d_h * quality2);

	if ((dw2 != d_w2) || (dh2 != d_h2)) {
		d_w2 = dw2;
		d_h2 = dh2;

		_InitGBuffer(texs2, dw2, dh2);
	}
}

void Camera::InitGBuffer(uint w, uint h) {
	_InitGBuffer(texs, (float)w, (float)h);

	glGenFramebuffers(NUM_EXTRA_TEXS, blitFbos);
	glGenTextures(NUM_EXTRA_TEXS, blitTexs);

	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	for (byte i = 0; i < NUM_EXTRA_TEXS; ++i) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, blitFbos[i]);
		glBindTexture(GL_TEXTURE_2D, blitTexs[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)Display::width, (int)Display::height, 0, GL_RGBA, GL_FLOAT, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blitTexs[i], 0);
		SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDrawBuffers(1, DrawBuffers);
		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (Status != GL_FRAMEBUFFER_COMPLETE) {
			Debug::Error("Camera", "FB error " + std::to_string(i) + ":" + std::to_string(Status));
		}
	}


	glGenFramebuffers(1, &trTexs.fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, trTexs.fbo);

	glGenTextures(1, &trTexs.colTex);
	glGenTextures(1, &trTexs.depthTex);

	glBindTexture(GL_TEXTURE_2D, trTexs.colTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (int)w, (int)h, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, trTexs.colTex, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, trTexs.depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, (int)w, (int)h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, trTexs.depthTex, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

	glDrawBuffers(1, DrawBuffers);
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		Debug::Error("Camera", "Overlay FB error");
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


GLuint Camera::emptyVao = 0;
//const float Camera::fullscreenVerts[] = { -1, -1, 1,  -1, 1, 1,  1, -1, 1,  1, 1, 1 };
const float Camera::fullscreenVerts[] = { -1, -1,  -1, 1,  1, -1,  1, 1 };
const int Camera::fullscreenIndices[] = { 0, 1, 2, 1, 3, 2 };
GLuint Camera::rectIdBuf = 0;

uint Camera::GetIdAt(uint x, uint y) {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, texs.fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	uint pixel[2];
	x = uint(x * quality * Display::dpiScl);
	y = uint(y * quality * Display::dpiScl);
	glReadPixels(x, d_h - y - 1, 1, 1, GL_RG_INTEGER, GL_UNSIGNED_INT, pixel);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	if (pixel[1] != 0) return 0;
	return pixel[0];
}

void Camera::Render(onBlitFunc func) {
	active = this;
	uint t_w = (uint)roundf(Display::width * quality);
	uint t_h = (uint)roundf(Display::height * quality);
	if ((d_w != t_w) || (d_h != t_h)) {
		if (!!texs.fbo) {
			texs.Clear();
			glDeleteFramebuffers(NUM_EXTRA_TEXS, blitFbos);
			glDeleteTextures(NUM_EXTRA_TEXS, blitTexs);
			trTexs.Clear();
		}
		InitGBuffer(t_w, t_h);
		_texs = texs;
		d_w = t_w;
		d_h = t_h;
		_d_w = d_w;
		_d_h = d_h;
		Scene::dirty = true;
		_w = Display::width;
		_h = Display::height;
	}
	if (useGBuffer2) {
		GenGBuffer2();
		if (applyGBuffer2) {
			d_w = d_w2;
			d_h = d_h2;
			texs = texs2;
			Display::width = (uint)(_w * quality2);
			Display::height = (uint)(_h * quality2);
		}
	}

	float zero[4] = {};
	float one = 1;
	if (Scene::dirty) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, texs.fbo);
		glClearBufferfv(GL_COLOR, 0, zero);
		glClearBufferfv(GL_COLOR, 1, zero);
		glClearBufferfv(GL_COLOR, 2, zero);
		glClearBufferfv(GL_DEPTH, 0, &one);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(true);
		glDisable(GL_BLEND);
		MVP::Switch(true);
		MVP::Clear();
		MVP::Translate(-1 - offset.x * 2, 1 + offset.y * 2, 0);
		MVP::Scale(scale, scale, 1);
		MVP::Translate(1, -1, 0);
		ApplyGL();
		MVP::Switch(false);
		MVP::Clear();
		glViewport(0, 0, d_w, d_h);

		func();

		glViewport(0, 0, Display::frameWidth, Display::frameHeight);
	}
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	if (useGBuffer2 && applyGBuffer2) {
		Display::width = _w;
		Display::height = _h;
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	onBlit();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	if (useGBuffer2 && applyGBuffer2) {
		d_w = _d_w;
		d_h = _d_h;
		texs = _texs;
	}

	Scene::dirty = false;
}

GLuint Light::_shadowFbo = 0;
GLuint Light::_shadowGITexs[] = {0, 0, 0};
GLuint Light::_shadowMap = 0;
GLuint Light::_shadowCubeFbos[] = { 0, 0, 0, 0, 0, 0 };
GLuint Light::_shadowCubeMap = 0;
GLuint Light::_fluxFbo = 0;
GLuint Light::_fluxTex = 0;
GLuint Light::_rsmFbo = 0;
GLuint Light::_rsmTex = 0;
GLuint Light::_rsmUBO = 0;
RSM_RANDOM_BUFFER Light::_rsmBuffer = RSM_RANDOM_BUFFER();
std::vector<GLint> Light::paramLocs_Point = std::vector<GLint>();
std::vector<GLint> Light::paramLocs_Spot = std::vector<GLint>();
std::vector<GLint> Light::paramLocs_SpotCS = std::vector<GLint>();
std::vector<GLint> Light::paramLocs_SpotFluxer = std::vector<GLint>();
std::vector<GLint> Light::paramLocs_SpotRSM = std::vector<GLint>();

void Light::InitShadow() {
	glGenFramebuffers(1, &_shadowFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowFbo);

	//glGenTextures(3, _shadowGITexs);
	glGenTextures(1, &_shadowMap);

	for (uint i = 0; i < 3; ++i) {
		glBindTexture(GL_TEXTURE_2D, _shadowGITexs[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _shadowGITexs[i], 0);
		SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	}

	// depth
	glBindTexture(GL_TEXTURE_2D, _shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMap, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		Debug::Error("ShadowMap", "FB error:" + std::to_string(Status));
		abort();
	}
	else {
		Debug::Message("ShadowMap", "FB ok");
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Light::DrawShadowMap(GLuint tar) {
	if (_shadowFbo == 0) {
		Debug::Error("RenderShadow", "Fatal: Fbo not set up!");
	}
	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, tar);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowFbo);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);
	glDisable(GL_BLEND);
	float zero[4] = {};
	float one = 1;
	glClearBufferfv(GL_COLOR, 0, zero);
	glClearBufferfv(GL_DEPTH, 0, &one);
	MVP::Switch(false);
	MVP::Clear();

	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);

	glEnable(GL_BLEND);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, tar);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tar);
}