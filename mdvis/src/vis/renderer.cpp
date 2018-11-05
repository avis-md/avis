#include "renderer.h"
#include "pargraphics.h"
#include "gif/gif.h"
#include "utils/avi.h"
#include "md/parmenu.h"
#include "ui/ui_ext.h"
#include "ui/localizer.h"
#include "utils/tinyfiledialogs.h"

VisRenderer::IMG_TYPE VisRenderer::imgType;
VisRenderer::VID_TYPE VisRenderer::vidType;
VisRenderer::STATUS VisRenderer::status = STATUS::READY;

#define RESO 2048
#define VRESO 1024

bool VisRenderer::imgUseAlpha = true;
uint VisRenderer::imgW = RESO, VisRenderer::imgH = RESO, VisRenderer::vidW = VRESO, VisRenderer::vidH = VRESO;
uint VisRenderer::imgSlices = 4;
uint VisRenderer::imgMsaa = 4, VisRenderer::vidMsaa = 4;
uint VisRenderer::vidMaxFrames = 1000;
float VisRenderer::resLerp = -1;

std::string VisRenderer::outputFolder =
#ifdef PLATFORM_WIN
"C:/tmp/avis/";
#else
"/var/tmp/avis/";
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
	if (status == STATUS::IMG) {
		UI::IncLayer();
		UI::Quad(0, 0, static_cast<float>(Display::width), static_cast<float>(Display::height), black(0.9f*resLerp));
		resLerp = (resLerp >= 0)? std::min(resLerp + 4 * Time::delta, 1.f) : 0;
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
			const char* fmt[] = { "*.png" };
			auto path = tinyfd_saveFileDialog("Save Image", nullptr, 0, fmt, "image file");
			if (path) {
				glBindFramebuffer(GL_READ_FRAMEBUFFER, res_fbo);
				std::vector<byte> res(imgW * imgH * 4);
				glReadPixels(0, 0, imgW, imgH, GL_RGBA, GL_UNSIGNED_BYTE, &res[0]);
				Texture::ToPNG(res, imgW, imgH, std::string(path) + ".png");
				glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
				status = STATUS::READY;
				resLerp = -1;
				Scene::dirty = true;
			}
		}
		if (Input::KeyDown(Key_Escape)) {
			status = STATUS::READY;
			resLerp = -1;
			Scene::dirty = true;
		}
	}
}

void VisRenderer::DrawMenu() {
	auto& ep = ParMenu::expandPos;
	float off = 20;

	UI::Label(ep - 148, off, 12, _("Image (GLSL)"), white());
	off += 17;
	UI::Quad(ep - 149, off - 1, 148, 17 * 5 + 3, white(0.9f, 0.1f));
	if (Engine::Button(ep - 147, off, 145, 16, Vec4(0.2f, 0.4f, 0.2f, 1), _("Render"), 12, white(), true) == MOUSE_RELEASE) {
		ToImage();
	}
	off += 18;
	imgW = TryParse(UI2::EditText(ep - 147, off, 146, _("Width"), std::to_string(imgW)), 1024U);
	off += 17;
	imgH = TryParse(UI2::EditText(ep - 147, off, 146, _("Height"), std::to_string(imgH)), 1024U);
	off += 17;
	imgSlices = TryParse(UI2::EditText(ep - 147, off, 146, _("Slices"), std::to_string(imgSlices)), 1024U);
	off += 17;
	bool ms = !!imgMsaa;
	UI2::Toggle(ep - 147, off, 145, _("MSAA"), ms);
	imgMsaa = ms? 4 : 0;
	off += 20;

	UI::Label(ep - 148, off, 12, _("Movie (GLSL)"), white());
	off += 17;
	UI::Quad(ep - 149, off - 1, 148, 17 * 6 + 3, white(0.9f, 0.1f));
	if (Engine::Button(ep - 147, off, 145, 16, Vec4(0.2f, 0.4f, 0.2f, 1), _("Render"), 12, white(), true) == MOUSE_RELEASE) {
		ToVid();
	}
	off += 18;
	static std::string fmts[] = { "AVI", "GIF", "PNG Sequence", "" };
	static Popups::DropdownItem di((uint*)&vidType, fmts);
	UI2::Dropdown(ep - 147, off, 146, _("Format"), di);
	off += 17;
	vidW = TryParse(UI2::EditText(ep - 147, off, 146, _("Width"), std::to_string(vidW)), 1024U);
	off += 17;
	vidH = TryParse(UI2::EditText(ep - 147, off, 146, _("Height"), std::to_string(vidH)), 1024U);
	off += 17;
	ms = !!vidMsaa;
	UI2::Toggle(ep - 147, off, 145, _("MSAA"), ms);
	vidMsaa = ms? 4 : 0;
	off += 17;
	vidMaxFrames = TryParse(UI2::EditText(ep - 147, off, 146, _("Max Frames"), std::to_string(vidMaxFrames)), 1000U);
}

void VisRenderer::ToImage() {
	if (status != STATUS::READY) return;
	status = STATUS::BUSY;

	Debug::Message("Renderer::ToImage", "Rendering image");
	int iw = imgW / imgSlices;
	int ih = imgH / imgSlices;
	if ((iw * imgSlices != imgW) || (ih * imgSlices != imgH))
		Debug::Warning("VisRenderer", "Image cannot be cleanly sliced into " + std::to_string(imgSlices) + "!");

	auto& cam = ChokoLait::mainCamera;
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
	if (imgSlices > 1 || imgMsaa > 0) {
		MakeTex(tmp_fbo, tmp_img, iw, ih);
		cam->target = tmp_fbo;
	}
	else {
		cam->target = res_fbo;
	}
	
	cam->scale = imgSlices;
	for (int a = 0; a < imgSlices; ++a) {
		for (int b = 0; b < imgSlices; ++b) {
			if (imgMsaa > 0) {
				const float ctw = 0.5f / iw;
				const float cth = 0.5f / ih;
				const Vec2 smps[] = { Vec2(ctw / 2, cth / 2), Vec2(3 * ctw / 2, cth / 2), Vec2(ctw / 2, 3 * cth / 2), Vec2(3 * ctw / 2, 3 * cth / 2) };
				for (int c = 0; c < 4; ++c) {
					Scene::dirty = true;
					cam->offset = Vec2(a, b) + Vec2(smps[c]);
					cam->Render([]() {
						auto& cam = ChokoLait::mainCameraObj->transform;
						ParGraphics::Rerender(cam.position(), cam.forward(), (float)imgW / imgSlices, (float)imgH / imgSlices);
					});
					MVP::Switch(false);
					MVP::Clear();
					glDepthMask(true);
					glViewport(0, 0, imgW, imgH);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, res_fbo);
					const float dis = 1.f / imgSlices;
					if (c > 0) glBlendFunc(GL_ONE, GL_ONE);
					else glBlendFunc(GL_ONE, GL_ZERO);
					UI::Quad(a*iw*dis, b*ih*dis, iw*dis, ih*dis, tmp_img, white()*0.25f);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				}
			}
			else {
				Scene::dirty = true;
				cam->offset = Vec2(a, b);
				cam->Render([]() {
					auto& cam = ChokoLait::mainCameraObj->transform;
					ParGraphics::Rerender(cam.position(), cam.forward(), (float)imgW / imgSlices, (float)imgH / imgSlices);
				});
				if (imgSlices > 1) {
					MVP::Switch(false);
					MVP::Clear();
					glDepthMask(true);
					glViewport(0, 0, imgW, imgH);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, res_fbo);
					const float dis = 1.f / imgSlices;
					glBlendFunc(GL_ONE, GL_ZERO);
					UI::Quad(a*iw*dis, b*ih*dis, iw*dis, ih*dis, tmp_img, white());
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				}
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

	status = STATUS::IMG;
}

void VisRenderer::ToVid() {
	if (status != STATUS::READY) return;
	status = STATUS::BUSY;
	Debug::Message("Renderer::ToVid", "Starting");
	
	auto vidSkip = std::max(Particles::anim.frameCount/vidMaxFrames, 1U);
	
	static Texture wtx(IO::path + "res/cat2.jpg");
	UI::Quad(0, 0, Display::width, Display::height, black());
	UI::Label(10, 10, 15, "Rendering, please wait...", white());
	UI::Label(10, 30, 15, "Unfortunately, the UI during rendering is not implemented yet.", white());
	UI::Label(10, 50, 15, "So look at this cat instead.", white());
	UI::Texture(10, 30, Display::height*0.5f, Display::height*0.5f, wtx, DRAWTEX_FIT);
	glfwSwapBuffers(ChokoLait::window);

	auto& cam = ChokoLait::mainCamera;
	auto w = Display::width;
	auto h = Display::height;
	auto w2 = Display::actualWidth;
	auto h2 = Display::actualHeight;
	auto w3 = Display::frameWidth;
	auto h3 = Display::frameHeight;
	auto frm = Particles::anim.currentFrame;
	Display::width = Display::actualWidth = Display::frameWidth = vidW;
	Display::height = Display::actualHeight = Display::frameHeight = vidH;
	ParGraphics::hlIds.clear();
	if (imgUseAlpha) ParGraphics::bgCol.a = 0;

	MakeTex(tmp_fbo, tmp_img, vidW, vidH);
	if (vidMsaa > 0) {
		MakeTex(res_fbo, res_img, vidW, vidH);
		cam->target = tmp_fbo;
	}
	else {
		cam->target = res_fbo;
	}
	cam->offset = Vec2();
	cam->scale = 1;
	const float ctw = 0.25f / vidW;
	const float cth = 0.25f / vidH;
	const Vec2 smps[] = { Vec2(-ctw, -cth), Vec2(ctw, -cth), Vec2(-ctw, cth), Vec2(ctw, cth) };
	
	auto delay = (uint32_t)std::ceil(1.0f / ParGraphics::animTarFps);

	AVI avifile;
	GifWriter giffile;
	switch (vidType) {
	case VID_TYPE::AVI:
		avifile = AVI(IO::currPath + "movie.avi", vidW, vidH, 30);
		break;
	case VID_TYPE::GIF:
		GifBegin(&giffile, (IO::currPath + "movie.gif").c_str(), vidW, vidH, delay);
		break;
	default:
		IO::MakeDirectory(IO::currPath + "renderSequence/");
		break;
	}
	std::vector<byte> res(vidW * vidH * 4);

	int _f = 0;
	for (uint f = 0; f < Particles::anim.frameCount; f += vidSkip) {
		Debug::Message("Renderer::ToVid", "Rendering frame " + std::to_string(f));
		Particles::SetFrame(f);
		Particles::Update();
		if (vidMsaa > 0) {
			for (int c = 0; c < 4; ++c) {
				Scene::dirty = true;
				cam->offset = Vec2(smps[c]);
				cam->Render([]() {
					auto& cam = ChokoLait::mainCameraObj->transform;
					ParGraphics::Rerender(cam.position(), cam.forward(), vidW, vidH);
				});
				MVP::Switch(false);
				MVP::Clear();
				glDepthMask(true);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, res_fbo);
				if (c > 0) glBlendFunc(GL_ONE, GL_ONE);
				else glBlendFunc(GL_ONE, GL_ZERO);
				UI::Quad(0, 0, vidW, vidH, tmp_img, white()*0.25f);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			}
		}
		else {
			Scene::dirty = true;
			cam->Render([]() {
				auto& cam = ChokoLait::mainCameraObj->transform;
				ParGraphics::Rerender(cam.position(), cam.forward(), vidW, vidH);
			});
		}
		
		glfwPollEvents();

		switch (vidType) {
		case VID_TYPE::AVI:
			avifile.AddFrame(res_img);
			break;
		case VID_TYPE::GIF:
			glBindFramebuffer(GL_READ_FRAMEBUFFER, res_fbo);
			glReadPixels(0, 0, vidW, vidH, GL_RGBA, GL_UNSIGNED_BYTE, &res[0]);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glFinish();
			GifWriteFrame(&giffile, &res[0], vidW, vidH, delay);
			break;
		default:
			std::vector<byte> vres;
			vres.resize(vidW*vidH*4);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, res_fbo);
			glReadPixels(0, 0, vidW, vidH, GL_RGBA, GL_UNSIGNED_BYTE, &vres[0]);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glFinish();
			Texture::ToPNG(vres, vidW, vidH, IO::currPath + "renderSequence/frame" + std::to_string(_f) + ".png");
			_f++;
			break;
		}
	}

	switch (vidType) {
	case VID_TYPE::AVI:
		avifile.End();
		break;
	case VID_TYPE::GIF:
		GifEnd(&giffile);
		break;
	default:
		break;
	}
	Debug::Message("Renderer::ToVid", "Finished");

	Display::width = w;
	Display::height = h;
	Display::actualWidth = w2;
	Display::actualHeight = h2;
	Display::frameWidth = w3;
	Display::frameHeight = h3;
	Particles::SetFrame(frm);
	cam->target = 0;
	ParGraphics::bgCol.a = 1;
	Scene::dirty = true;
	status = STATUS::READY;
}

void VisRenderer::MakeTex(GLuint& fbo, GLuint& tex, int w, int h) {
	if (fbo) {
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &tex);
	}
	glGenTextures(1, &tex);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		Debug::Error("VisRend", "FB error:" + std::to_string(status));
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
