#define _CRT_SECURE_NO_WARNINGS
#include "renderer.h"
#include "pargraphics.h"

VisRenderer::IMG_TYPE VisRenderer::imgType;
VisRenderer::VID_TYPE VisRenderer::vidType;
VisRenderer::STATUS VisRenderer::status;

bool VisRenderer::imgUseAlpha = true, VisRenderer::vidUseAlpha;
uint VisRenderer::imgW = 2048, VisRenderer::imgH = 2048, VisRenderer::vidW = 1024, VisRenderer::vidH = 600;
float VisRenderer::resLerp = -1;

std::string VisRenderer::outputFolder =
#ifdef PLATFORM_WIN
"C:/tmp/mdvis/";
#else
"/var/tmp/mdvis/";
#endif
;

GLuint VisRenderer::res_fbo = 0, VisRenderer::res_img = 0;

void VisRenderer::Draw() {
	if (status == IMG) {
		UI::IncLayer();
		UI::Quad(0, 0, (float)Display::width, (float)Display::height, black(0.9f*resLerp));
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
	_SetRes();

	auto cam = ChokoLait::mainCamera();
	Scene::dirty = true;
	auto w = Display::width;
	auto h = Display::height;
	auto w2 = Display::actualWidth;
	auto h2 = Display::actualHeight;
	Display::width = Display::actualWidth = imgW;
	Display::height = Display::actualHeight = imgH;
	cam->target = res_fbo;
	ParGraphics::hlIds.clear();
	if (imgUseAlpha) ParGraphics::bgCol.a = 0;

	cam->Render([]() {
		auto& cm = ChokoLait::mainCamera->object->transform;
		ParGraphics::Rerender(cm.position(), cm.forward(), (float)imgW, (float)imgH);
	});

	Display::width = w;
	Display::height = h;
	Display::actualWidth = w2;
	Display::actualHeight = h2;
	cam->target = 0;
	ParGraphics::bgCol.a = 1;
	Scene::dirty = true;

	status = IMG;
}

void VisRenderer::_SetRes() {
	if (res_fbo) {
		glDeleteFramebuffers(1, &res_fbo);
	}
	else {
		glGenTextures(1, &res_img);
	}
	glGenFramebuffers(1, &res_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, res_fbo);

	glBindTexture(GL_TEXTURE_2D, res_img);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgW, imgH, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, res_img, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		Debug::Error("VisRend", "FB error:" + std::to_string(status));
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}