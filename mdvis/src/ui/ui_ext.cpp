#include "ui_ext.h"
#include "icons.h"
#include "popups.h"
#include "utils/dialog.h"
#include "res/shddata.h"

PROGDEF(UI2::bezierProg)

void UI2::Init() {
	bezierProg = Shader::FromVF(glsl::bezierVert, glsl::coreFrag3);
#define LC(nm) bezierProgLocs[i++] = glGetUniformLocation(bezierProg, #nm) 
	int i = 0;
	LC(pts);
	LC(count);
	LC(col);
#undef LC
}

void UI2::LabelMul(float x, float y, float sz, const std::string& s) {
	auto ss = string_split(s, '\n');
	for (size_t a = 0; a < ss.size(); a++) {
		UI::Label(x, round(y + sz * 1.2f * a), sz, ss[a], white());
	}
}

std::string UI2::EditText(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled, Vec4 col) {
	UI::Label(x, y, 12, title, white());
	w /= 2;
	if (enabled) {
		return UI::EditText(x + w, y, w - 1.0f, 16, 12, col, val, true, white());
	}
	else {
		Engine::Button(x + w, y, w - 1.0f, 16, col, val, 12, white(0.5f));
		return val;
	}
}

float UI2::Slider(float x, float y, uint w, const std::string& title, float a, float b, float t) {
	return Slider(x, y, w, title, a, b, t, std::to_string(t));
}

float UI2::Slider(float x, float y, uint w, const std::string& title, float a, float b, float t, const std::string& lbl) {
	UI::Label(x, y, 12, title, white());
	w /= 2;
	t = Engine::DrawSliderFill(x + w, y, w - 1.0f, 16, a, b, t, white(1, 0.5f), white());
	UI::Label(x + w + 2, y, 12, lbl, white(1, 0.2f));
	return t;
}

void UI2::Color(float x, float y, uint w, const std::string& title, Vec4& col) {
	UI::Label(x, y, 12, title, white());
	w /= 2;
	if (Engine::Button(x + w, y, w-1.0f, 16, col) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::COLORPICK;
		Popups::pos = Vec2(x + w, y + 16);
		Popups::data = &col;
	}
	UI::Texture(x + w * 2 - 18, y, 16, 16, Icons::colorwheel);
}

void UI2::File(float x, float y, uint w, const std::string& title, const std::string& fl, std::function<void(std::vector<std::string>)> func) {
	UI::Label(x, y, 12, "File", white());
	w /= 2;
	if (Engine::Button(x + w, y, w-1.0f, 16, white(1, 0.3f), fl, 12, white(0.5f)) == MOUSE_RELEASE) {
		std::vector<std::string> exts = {"*.hdr"};
		auto res = Dialog::OpenFile(exts);
		if (!!res.size()) {
			func(res);
		}
	}
}

MOUSE_STATUS UI2::Button2(float x, float y, float w, const std::string& s, Texture* tex, Vec4 col, Vec4 col2) {
	auto ret = Engine::Button(x, y, w, 16, col);
	UI::Texture(x + 1, y, 16, 16, tex, col2);
	UI::font->alignment = ALIGN_TOPCENTER;
	UI::Label(x + 9 + w/2, y, 12, s, col2);
	UI::font->alignment = ALIGN_TOPLEFT;
	return ret;
}

void UI2::Dropdown(float x, float y, float w, const std::string& title, const Popups::DropdownItem& data) {
	UI::Label(x, y, 12, title, white());
	w /= 2;
	if (Engine::Button(x + w, y, w - 1, 16, white(1, 0.3f), data.list[*data.target], 12, white()) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::DROPDOWN;
		Popups::pos = Vec2(x + w, y + 16);
		Popups::pos2.x = w - 1;
		Popups::data = (Popups::DropdownItem*)&data;
	}
	UI::Texture(x + w * 2 - 16, y, 16, 16, Icons::dropdown2);
}

void UI2::Toggle(float x, float y, float w, const std::string& title, bool& val) {
	UI::Label(x, y, 12, title, white());
	val = Engine::Toggle(x + w - 16, y, 16, Icons::checkbox, val, white(), ORIENT_HORIZONTAL);
}

void UI2::Switch(float x, float y, float w, const std::string& title, int c, std::string* nms, int& i) {
	UI::Label(x, y, 12, title, white());
	x += w/3;
	w *= 0.67f;
	float dw = w / c;
	for (int a = 0; a < c; a++) {
		if (Engine::Button(x + dw * a, y, dw - 1, 16, white(1, (a == i) ? 0.1f : 0.3f), nms[a], 12, white(), true) == MOUSE_RELEASE) {
			i = a;
		}
	}
}

void UI2::Bezier(Vec2 p1, Vec2 t1, Vec2 t2, Vec2 p2, Vec4 col, int reso) {
	glUseProgram(bezierProg);
	Vec2 pts[4] = { Ds2(p1), Ds2(t1), Ds2(t2), Ds2(p2) };
	glUniform2fv(bezierProgLocs[0], 4, &pts[0][0]);
	glUniform1i(bezierProgLocs[1], reso-1);
	glUniform4f(bezierProgLocs[2], col.r, col.g, col.b, col.a);
	glBindVertexArray(Camera::emptyVao);
	glDrawArrays(GL_LINE_STRIP, 0, reso);
	glUseProgram(0);
}