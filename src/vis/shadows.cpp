#include "shadows.h"
#include "pargraphics.h"
#include "md/parmenu.h"
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/shd/minVert.h"

pCamera Shadows::cam;

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

bool Shadows::isPass = false;

GLuint Shadows::_fbo, Shadows::_dtex;

Shader Shadows::_prog;

void Shadows::Init() {
	cam = ChokoLait::mainCamera;

	_prog = Shader::FromVF(glsl::minVert, IO::GetText(IO::path + "shadows.txt"));

#define LOC(nm) _prog.AddUniform(#nm)
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
	SetTexParams<>(0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

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
	for (uint a = 0; a < 8; ++a) {
		wps[a] = ip * es[a];
		wps[a] /= wps[a].w;
	}
	for (uint a = 4; a < 8; ++a) {
		wps[a] = Lerp(wps[a - 4], wps[a], dst);
	}
	pos = Vec3();
		//pos += *(Vec3*)&wps[a];
	pos /= 8.f;
	memset(box, 0, sizeof(float) * 6);
	for (uint a = 0; a < 8; ++a) {
		Vec4 rs = _p * wps[a];
		rs /= rs.w;
		box[0] = std::min(box[0], rs.x);
		box[1] = std::max(box[1], rs.x);
		box[2] = std::min(box[2], rs.y);
		box[3] = std::max(box[3], rs.y);
		box[4] = std::min(box[4], rs.z);
		box[5] = std::max(box[5], rs.z);
	}
	*/
	_p = glm::ortho<float>(-1, 1, -1, 1, 0.01f, 100);//glm::ortho(box[0], box[1], box[2], box[3], box[4] - dst2, box[5]) * _p;
	_p *= Mat4x4(1.f, 0, 0, 0, 0, 1.f, 0, 0, 0, 0, -1.f, 0, 0, 0, 0, 1);
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
	isPass = true;
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
	float one = 1;
	glClearBufferfv(GL_DEPTH, 0, &one);
	MVP::Switch(true);
	MVP::Clear();
	MVP::Mul(_p);

	MVP::Switch(false);
	MVP::Clear();
	glViewport(0, 0, _sz, _sz);
	//ParGraphics::Rerender(cpos, glm::normalize(pos - cpos), (float)_sz, (float)_sz);
	glViewport(0, 0, Display::frameWidth, Display::frameHeight);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cam->texs.fbo);
	isPass = false;
}

void Shadows::Reblit() {
	auto _cp = MVP::projection();

	glUseProgram(_prog);
	glUniformMatrix4fv(_prog.Loc(0), 1, GL_FALSE, glm::value_ptr(glm::inverse(_cp)));
	glUniform2f(_prog.Loc(1), (float)Display::actualWidth, (float)Display::actualHeight);
	glUniform1i(_prog.Loc(2), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cam->texs.normTex);
	glUniform1i(_prog.Loc(3), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cam->texs.depthTex);
	glUniform1i(_prog.Loc(4), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _dtex);
	glUniform1f(_prog.Loc(5), bias / 100);
	glUniform1f(_prog.Loc(6), str);
	glUniform3f(_prog.Loc(7), cpos.x, cpos.y, cpos.z);
	glUniform1f(_prog.Loc(8), _sz / 2048.f);
	glUniformMatrix4fv(_prog.Loc(9), 1, GL_FALSE, glm::value_ptr(_p));

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
#undef SV

	UI::Quad(expandPos - 148, off - 1, 147, 17 * 5 + 2, white(0.9f, 0.1f));
	UI::Label(expandPos - 147, off, 12, "Shadows", white());
	show = Engine::Toggle(expandPos - 19, off, 16, Icons::checkbox, show, white(), ORIENT_HORIZONTAL);
	str = UI2::Slider(expandPos - 147, off + 17, 146, "Strength", 0, 1, str);
	bias = UI2::Slider(expandPos - 147, off + 17 * 2, 146, "Bias", 0, 1, bias);
	rw = UI2::Slider(expandPos - 147, off + 17 * 3, 146, "Angle W", 0, 360, rw);
	rz = UI2::Slider(expandPos - 147, off + 17 * 4, 146, "Angle Y", -90, 90, rz);
#define DF(x) (_ ## x != x)
	if (DF(show) || DF(str) || DF(bias) || DF(rw) || DF(rz)) {
		Scene::dirty = true;
	}
#undef DF

	return off + 17 * 5 + 1;
}

void Shadows::Serialize(XmlNode* nd) {
#define SVS(nm, vl) nd->addchild(#nm, vl)
#define SV(nm, vl) SVS(nm, std::to_string(vl))
	SVS(enabled, show? "1" : "0");
	SV(quality, quality); SV(strength, str);
	SV(bias, bias); SV(rw, rw); SV(rz, rz);
#undef SVS
#undef SV
}

void Shadows::Deserialize(XmlNode* nd) {
#define GT(nm, vl) if (n.name == #nm) vl = TryParse(n.value, vl)
#define GTV(nm, vl) if (n.name == #nm) Xml::ToVec(&n, vl)
	for (auto& n : nd->children) {
		if (n.name == "enabled") show = (n.value == "1");
		else GT(quality, quality);
		else GT(strength, str);
		else GT(bias, bias);
		else GT(rw, rw);
		else GT(rz, rz);
	}
#undef GT
#undef GTV
}