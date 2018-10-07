#define _CRT_SECURE_NO_WARNINGS
#include "renderer.h"
#include "pargraphics.h"

VisRenderer::IMG_TYPE VisRenderer::imgType;
VisRenderer::VID_TYPE VisRenderer::vidType;
VisRenderer::STATUS VisRenderer::status;

#define RESO 2048*8

bool VisRenderer::imgUseAlpha = false, VisRenderer::vidUseAlpha;
uint VisRenderer::imgW = RESO, VisRenderer::imgH = RESO, VisRenderer::vidW = 1024, VisRenderer::vidH = 600;
uint VisRenderer::imgSlices = 4;
float VisRenderer::resLerp = -1;

std::string VisRenderer::outputFolder =
#ifdef PLATFORM_WIN
"C:/tmp/mdvis/";
#else
"/var/tmp/mdvis/";
#endif
;

GLuint VisRenderer::res_fbo = 0, VisRenderer::res_img = 0;
GLuint VisRenderer::tmp_fbo = 0, VisRenderer::tmp_img = 0;

int VisRenderer::_maxTexSz;

void VisRenderer::Init() {
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTexSz);
	if (!_maxTexSz) {
		Debug::Warning("VisRenderer", "Unable to determine max texture size, assuming 2048!");
		_maxTexSz = 2048;
	}
}

void VisRenderer::Draw() {
	if (status == IMG) {
		UI::IncLayer();
		UI::Quad(0, 0, static_cast<float>(Display::width), static_cast<float>(Display::height), black(0.9f*resLerp));
		resLerp = (resLerp >= 0)? min(resLerp + 4 * Time::delta, 1.0f) : 0;
		float dw = Display::width * 0.1f;
		float dh = Display::height * 0.1f;
		float whi = ((float)imgW) / imgH;
		float dwh = ((float)dw) / dh;
		if (dwh > whi) {
			auto hh = Display::height - 2 * dh;
			hh = hh * whi;
			dw = (Display::width - hh)/2;
		}
		else {
			auto ww = Display::width - 2 * dw;
			ww = ww / whi;
			dh = (Display::height - ww) / 2;
		}
		dw *= resLerp;
		dh *= resLerp;
		if (!imgUseAlpha) {
			glDisable(GL_BLEND);
		}
		UI::Quad(dw, dh, Display::width - 2 * dw, Display::height - 2 * dh, res_img, white(resLerp));
		if (!imgUseAlpha) {
			glEnable(GL_BLEND);
		}
		if (Engine::Button(Display::width - dw - 120, dh - 20, 120, 16, white(1, 0.4f), "Save", 12, white(), true) == MOUSE_RELEASE) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, res_fbo);
			std::vector<byte> res(imgW * imgH * 4);
			glReadPixels(0, 0, imgW, imgH, GL_RGBA, GL_UNSIGNED_BYTE, &res[0]);
			Texture::ToPNG(res, imgW, imgH, IO::path + "ss.png");
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			status = READY;
			resLerp = -1;
			Scene::dirty = true;
		}
		if (Input::KeyDown(Key_Escape)) {
			status = READY;
			resLerp =-1;
			Scene::dirty = true;
		}
	}
}

void VisRenderer::ToImage() {
	if (status != READY) return;
	int iw = imgW / imgSlices;
	int ih = imgH / imgSlices;
	if ((iw * imgSlices != imgW) || (ih * imgSlices != imgH))
		Debug::Warning("VisRenderer", "Image cannot be cleanly sliced into " + std::to_string(imgSlices) + "!");

	auto cam = ChokoLait::mainCamera();
	auto w = Display::width;
	auto h = Display::height;
	auto w2 = Display::actualWidth;
	auto h2 = Display::actualHeight;
	auto w3 = Display::frameWidth;
	auto h3 = Display::frameHeight;
	Display::width = Display::actualWidth = Display::frameWidth = iw;
	Display::height = Display::actualHeight = Display::frameHeight = ih;
	ParGraphics::hlIds.clear();
	if (imgUseAlpha) ParGraphics::bgCol.a = 0;

	MakeTex(res_fbo, res_img, imgW, imgH);
	if (imgSlices > 1) {
		MakeTex(tmp_fbo, tmp_img, iw, ih);
		cam->target = tmp_fbo;
	}
	else {
		cam->target = res_fbo;
	}
	
	cam->scale = imgSlices;
	for (int a = 0; a < imgSlices; a++) {
		for (int b = 0; b < imgSlices; b++) {
			Scene::dirty = true;
			cam->offset = Vec2(a, b);
			cam->Render([]() {
				auto& cm = ChokoLait::mainCamera->object->transform;
				ParGraphics::Rerender(cm.position(), cm.forward(), (float)imgW / imgSlices, (float)imgH / imgSlices);
			});
			MVP::Switch(false);
			MVP::Clear();
			glDepthMask(true);
			if (imgSlices > 1) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, tmp_fbo);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, res_fbo);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glBlitFramebuffer(0, 0, iw, ih, iw*a, ih*(imgSlices - b - 1), iw*(a+1)-1, ih*(imgSlices - b)-1, GL_COLOR_BUFFER_BIT, GL_LINEAR);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			}
		}
	}
	cam->scale = 1;
	cam->offset = Vec2();

	Display::width = w;
	Display::height = h;
	Display::actualWidth = w2;
	Display::actualHeight = h2;
	Display::frameWidth = w3;
	Display::frameHeight = h3;
	cam->target = 0;
	ParGraphics::bgCol.a = 1;
	Scene::dirty = true;

	status = IMG;
}

void VisRenderer::MakeTex(GLuint& fbo, GLuint& tex, int w, int h) {
	if (fbo) {
		glDeleteFramebuffers(1, &fbo);
	}
	else {
		glGenTextures(1, &tex);
	}
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	SetTexParams<>();
	glBindTexture(GL_TEXTURE_2D, 0);
	
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		Debug::Error("VisRend", "FB error:" + std::to_string(status));
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}