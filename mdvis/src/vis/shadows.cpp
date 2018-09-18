#include "shadows.h"
#include "pargraphics.h"
#include "md/ParMenu.h"
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/shddata.h"

Camera* Shadows::cam;

bool Shadows::show = false;
byte Shadows::quality = 2;
Vec3 Shadows::pos, Shadows::cpos = Vec3(0, 0, -1);
float Shadows::str = 1, Shadows::bias = 0.05f;
float Shadows::rw, Shadows::rz;
Quat Shadows::rot;
float Shadows::dst = 0.5f, Shadows::dst2 = 0;
float Shadows::box[] = {};
uint Shadows::_sz = 1024;
Mat4x4 Shadows::_p, Shadows::_ip;

GLuint Shadows::_fbo, Shadows::_dtex, Shadows::_prog;
GLint Shadows::_progLocs[] = {};

void Shadows::Init() {
	cam = ChokoLait::mainCamera().get();

	_prog = Shader::FromVF(glsl::minVert, IO::GetText(IO::path + "shadows.txt"));

#define LOC(nm) _progLocs[i++] = glGetUniformLocation(_prog, #nm)
	uint i = 0;
	LOC(_IP);
	LOC(screenSize);
	LOC(inNormal);
	LOC(inDepth);
	LOC(lDepth);
	LOC(lBias);
	LOC(lStrength);
	LOC(lPos);
	LOC(ldScl);
	LOC(_LD);
#undef LOC

	glGenFramebuffers(1, &_fbo);
	glGenTextures(1, &_dtex);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
	glBindTexture(GL_TEXTURE_2D, _dtex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _dtex, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	//GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	//glDrawBuffers(3, DrawBuffers);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		Debug::Error("ShadowMap", "FB error " + std::to_string(Status));
		abort();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//UpdateBox();
}

void Shadows::UpdateBox() {
	/*
	const float z = 1;// dst * 2 - 1;
	const Vec4 es[] = {
		Vec4(-1,-1,-1,1), Vec4(-1,1,-1,1) , Vec4(1,-1,-1,1) , Vec4(1,1,-1,1),
		Vec4(-1,-1,1,1), Vec4(-1,1,1,1) , Vec4(1,-1,1,1) , Vec4(1,1,1,1) };
	Vec4 wps[8];
	auto p = MVP::projection();
	auto ip = glm::inverse(p);
	for (uint a = 0; a < 8; a++) {
		wps[a] = ip * es[a];
		wps[a] /= wps[a].w;
	}
	for (uint a = 4; a < 8; a++) {
		wps[a] = Lerp(wps[a - 4], wps[a], dst);
	}
	pos = Vec3();
		//pos += *(Vec3*)&wps[a];
	pos /= 8.0f;
	memset(box, 0, sizeof(float) * 6);
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
	*/
	_p = glm::ortho<float>(-1, 1, -1, 1, 0.01f, 100);//glm::ortho(box[0], box[1], box[2], box[3], box[4] - dst2, box[5]) * _p;
	_p *= Mat4x4(1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, -1.0f, 0, 0, 0, 0, 1);
	_p *= Mat4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1);
	float csw = cos(rw*deg2rad);
	float snw = sin(rw*deg2rad);
	float csz = cos(-rz*deg2rad);
	float snz = sin(-rz*deg2rad);
	_p *= (Mat4x4(1, 0, 0, 0, 0, csz, snz, 0, 0, -snz, csz, 0, 0, 0, 0, 1) * Mat4x4(csw, 0, -snw, 0, 0, 1, 0, 0, snw, 0, csw, 0, 0, 0, 0, 1));
	_ip = glm::inverse(_p);
	Vec4 cc(0, 0, -1, 1);
	cc = _ip * cc;
	cc /= cc.w;
	cpos = *(Vec3*)&cc;
}

void Shadows::Rerender() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
	float one = 1;
	glClearBufferfv(GL_DEPTH, 0, &one);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	//glDepthMask(true);
	//glDisable(GL_BLEND);
	MVP::Switch(true);
	MVP::Clear();
	MVP::Mul(_p);

	MVP::Switch(false);
	MVP::Clear();
	glViewport(0, 0, _sz, _sz);
	ParGraphics::Rerender(cpos, glm::normalize(pos - cpos), (float)_sz, (float)_sz);
	glViewport(0, 0, Display::actualWidth, Display::actualHeight);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cam->d_fbo);
}

void Shadows::Reblit() {
	auto _cp = MVP::projection();

	glUseProgram(_prog);
	glUniformMatrix4fv(_progLocs[0], 1, GL_FALSE, glm::value_ptr(glm::inverse(_cp)));
	glUniform2f(_progLocs[1], (float)Display::actualWidth, (float)Display::actualHeight);
	glUniform1i(_progLocs[2], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cam->d_texs[1]);
	glUniform1i(_progLocs[3], 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cam->d_depthTex);
	glUniform1i(_progLocs[4], 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _dtex);
	glUniform1f(_progLocs[5], bias / 100);
	glUniform1f(_progLocs[6], str);
	glUniform3f(_progLocs[7], cpos.x, cpos.y, cpos.z);
	glUniform1f(_progLocs[8], _sz / 2048.0f);
	glUniformMatrix4fv(_progLocs[9], 1, GL_FALSE, glm::value_ptr(_p));

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(Camera::emptyVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glUseProgram(0);
}

float Shadows::DrawMenu(float off) {
	auto& expandPos = ParMenu::expandPos;

#define SV(x) auto _ ## x = x
	SV(show);
	SV(str);
	SV(bias);
	SV(rw);
	SV(rz);

	UI::Quad(expandPos - 148, off - 1, 147, 17 * 5 + 2, white(0.9f, 0.1f));
	UI::Label(expandPos - 147, off, 12, "Shadows", white());
	show = Engine::Toggle(expandPos - 19, off, 16, Icons::checkbox, show, white(), ORIENT_HORIZONTAL);
	str = UI2::Slider(expandPos - 147, off + 17, 147, "Strength", 0, 5, str);
	bias = UI2::Slider(expandPos - 147, off + 17 * 2, 147, "Bias", 0, 1, bias);
	rw = UI2::Slider(expandPos - 147, off + 17 * 3, 147, "Angle W", 0, 360, rw);
	rz = UI2::Slider(expandPos - 147, off + 17 * 4, 147, "Angle Y", -90, 90, rz);
#define DF(x) (_ ## x != x)
	if (DF(show) || DF(str) || DF(bias) || DF(rw) || DF(rz)) {
		Scene::dirty = true;
	}

	return off + 17 * 5 + 1;
}