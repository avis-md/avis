#include "shadows.h"
#include "pargraphics.h"
#include "md/ParMenu.h"
#include "ui/icons.h"

Camera* Shadows::cam;

bool Shadows::show = false;
Vec3 Shadows::pos;
float Shadows::str;
float Shadows::rw, Shadows::rz;
Quat Shadows::rot;
float Shadows::dst = 0.5f, Shadows::dst2 = 0.1f;
float Shadows::box[] = {};
Mat4x4 Shadows::_p, Shadows::_ip;

GLuint Shadows::_fbo, Shadows::_dtex;

void Shadows::Init() {
	cam = ChokoLait::mainCamera().get();

	glGenFramebuffers(1, &_fbo);
	glGenTextures(1, &_dtex);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
	glBindTexture(GL_TEXTURE_2D, _dtex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _dtex, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	//glDrawBuffers(3, DrawBuffers);

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
	//UpdateBox();
}

void Shadows::UpdateBox() {
	const float z = dst * 2 - 1;
	const Vec4 es[] = {
		Vec4(-1,-1,-1,1), Vec4(-1,1,-1,1) , Vec4(1,-1,-1,1) , Vec4(1,1,-1,1),
		Vec4(-1,-1,z,1), Vec4(-1,1,z,1) , Vec4(1,-1,z,1) , Vec4(1,1,z,1) };
	Vec4 wps[8];
	auto ip = glm::inverse(MVP::projection());
	pos = Vec3();
	for (uint a = 0; a < 8; a++) {
		wps[a] = ip * es[a];
		wps[a] /= wps[a].w;
		pos += *(Vec3*)&wps[a];
	}
	pos /= 8.0f;

	float csz = cos(-rz*deg2rad);
	float snz = sin(-rz*deg2rad);
	float csw = cos(rw*deg2rad);
	float snw = sin(rw*deg2rad);
	_p = Mat4x4(1, 0, 0, 0, 0, csw, snw, 0, 0, -snw, csw, 0, 0, 0, 0, 1) * Mat4x4(csz, 0, -snz, 0, 0, 1, 0, 0, snz, 0, csz, 0, 0, 0, 0, 1);
	_p *= Mat4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -pos.x, -pos.y, -pos.z, 1);
	
	for (uint a = 0; a < 8; a++) {
		Vec4 rs = _p * wps[a];
		rs /= rs.w;
		box[0] = min(box[0], rs.x);
		box[1] = max(box[1], rs.x);
		box[2] = min(box[2], rs.y);
		box[3] = max(box[3], rs.y);
		box[4] = min(box[4], rs.z);
		box[5] = max(box[5], rs.z);
	}

	_p = glm::ortho(box[0], box[1], box[2], box[3], box[4] - dst2, box[5]) * _p;
	_ip = glm::inverse(_p);
}

void Shadows::Rerender() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
	float one = 1;
	glClearBufferfv(GL_DEPTH, 0, &one);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);
	glDisable(GL_BLEND);
	MVP::Switch(true);
	MVP::Clear();
	MVP::Mul(_p);
	MVP::Switch(false);
	MVP::Clear();
	glViewport(0, 0, 1024, 1024);
	ParGraphics::Rerender();
	glViewport(0, 0, Display::actualWidth, Display::actualHeight);
}

void Shadows::Reblit() {

}

float Shadows::DrawMenu(float off) {
	auto& expandPos = ParMenu::expandPos;

	UI::Label(expandPos - 148, off, 12, "Effects", white());

	off += 17;
	Engine::DrawQuad(expandPos - 148, off, 146, 17 * 5, white(0.9f, 0.1f));
	UI::Label(expandPos - 146, off, 12, "Shadows", white());
	show = Engine::Toggle(expandPos - 20, off, 16, Icons::checkbox, show, white(), ORIENT_HORIZONTAL);
	UI::Label(expandPos - 145, off + 17, 12, "Strength", white());
	str = Engine::DrawSliderFill(expandPos - 80, off + 17, 76, 16, 0, 1, str, white(1, 0.5f), white());
	UI::Label(expandPos - 145, off + 34, 12, "Angle x", white());
	rw = Engine::DrawSliderFill(expandPos - 80, off + 34, 76, 16, 0, 360, rw, white(1, 0.5f), white());
	UI::Label(expandPos - 145, off + 51, 12, "Angle y", white());
	rz = Engine::DrawSliderFill(expandPos - 80, off + 51, 76, 16, -90, 90, rz, white(1, 0.5f), white());
	UI::Label(expandPos - 145, off + 68, 12, "Distance", white());
	dst = Engine::DrawSliderFill(expandPos - 80, off + 68, 76, 16, 0, 2, dst, white(1, 0.5f), white());
	return off + 17 * 5 + 1;
}