#include "ui_ext.h"
#include "icons.h"
#include "popups.h"
#include "utils/dialog.h"
#include "res/shddata.h"

PROGDEF(UI2::bezierProg)

float UI2::sepw = 0.5f;
#define sepw2 (1-sepw)

void UI2::Init() {
	bezierProg = Shader::FromVF(glsl::bezierVert, glsl::coreFrag3);
#define LC(nm) bezierProgLocs[i++] = glGetUniformLocation(bezierProg, #nm) 
	int i = 0;
	LC(pts);
	LC(count);
	LC(thick);
	LC(col);
#undef LC
}

void UI2::LabelMul(float x, float y, float sz, const std::string& s) {
	auto ss = string_split(s, '\n');
	for (size_t a = 0; a < ss.size(); ++a) {
		UI::Label(x, std::roundf(y + sz * 1.2f * a), sz, ss[a], white());
	}
}

std::string UI2::EditText(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled, Vec4 col) {
	UI::Label(x, y, 12, title, white());
	if (enabled) {
		return UI::EditText(x + w*sepw, y, w*sepw2 - 1.f, 16, 12, col, val, true, white());
	}
	else {
		Engine::Button(x + w*sepw, y, w*sepw2 - 1.f, 16, col, val, 12, white(0.5f));
		return val;
	}
}

std::string UI2::EditPass(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled, Vec4 col) {
	UI::Label(x, y, 12, title, white());
	if (enabled) {
		return UI::EditTextPass(x + w*sepw, y, w*sepw2 - 1.f, 16, 12, col, val, '*', true, white());
	}
	else {
		Engine::Button(x + w*sepw, y, w*sepw2 - 1.f, 16, col, "****", 12, white(0.5f));
		return val;
	}
}

float UI2::Slider(float x, float y, float w, const std::string& title, float a, float b, float t) {
	return Slider(x, y, w, title, a, b, t, std::to_string(t));
}

float UI2::Slider(float x, float y, float w, const std::string& title, float a, float b, float t, const std::string& lbl) {
	UI::Label(x, y, 12, title, white());
	return Slider(x + w*sepw, y, w*sepw2 - 1.f, a, b, t);
}

float UI2::Slider(float x, float y, float w, float a, float b, float t) {
	t = Engine::DrawSliderFill(x, y, w, 16, a, b, t, white(1, 0.5f), white());
	UI::Label(x + 2, y, 12, std::to_string(t), white(1, 0.2f));
	return t;
}

void UI2::Color(float x, float y, float w, const std::string& title, Vec4& col) {
	UI::Label(x, y, 12, title, white());
	if (Engine::Button(x + w*sepw, y, w*sepw2 - 1.f, 16, col) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::COLORPICK;
		Popups::pos = Vec2(x + w*sepw, y + 16);
		Popups::data = &col;
	}
	UI::Texture(x + w - 18, y, 16, 16, Icons::colorwheel);
}

void UI2::File(float x, float y, float w, const std::string& title, const std::string& fl, filecallback func) {
	UI::Label(x, y, 12, "File", white());
	if (Engine::Button(x + w*sepw, y, w*sepw2 - 1.f, 16, white(1, 0.3f), fl, 12, white(0.5f)) == MOUSE_RELEASE) {
		std::vector<std::string> exts = {"*.hdr"};
		auto res = Dialog::OpenFile(exts);
		if (!!res.size()) {
			func(res);
		}
	}
}

MOUSE_STATUS UI2::Button2(float x, float y, float w, const std::string& s, const Texture& tex, Vec4 col, Vec4 col2) {
	auto ret = Engine::Button(x, y, w, 16, col);
	UI::Texture(x + 1, y, 16, 16, tex, col2);
	UI::font->alignment = ALIGN_TOPCENTER;
	UI::Label(x + 9 + w/2, y, 12, s, col2);
	UI::font->alignment = ALIGN_TOPLEFT;
	return ret;
}

void UI2::Dropdown(float x, float y, float w, const std::string& title, const Popups::DropdownItem& data) {
	UI::Label(x, y, 12, title, white());
	Dropdown(x + w*sepw, y, w*sepw2 - 1, data);
}

void UI2::Dropdown(float x, float y, float w, const Popups::DropdownItem& data, std::function<void()> func, std::string label, Vec4 col) {
	if (label[0] == 1) {
		if (!data.flags)
			label = data.list[*data.target];
		else {
			if (!*data.target) label = "None";
			else {
				int a = 0; int j = 0;
				while (data.list[a] != "") {
					if ((*data.target & (1 << a))>0) {
						if (!j) {
							label = data.list[a];
							j = 1;
						}
						else if (j < 3) {
							label += ", " + data.list[a];
							j++;
						}
						else {
							label += ", ...";
							break;
						}
					}
					a++;
				}
			}
		}
	}
	if (Engine::Button(x, y, w, 16, white(1, 0.3f), label, 12, col) == MOUSE_RELEASE) {
		if (func) func();
		Popups::type = POPUP_TYPE::DROPDOWN;
		Popups::pos = Vec2(x, y + 16);
		Popups::pos2.x = w;
		Popups::data = (Popups::DropdownItem*)&data;
	}
	UI::Texture(x + w - 16, y, 16, 16, Icons::dropdown2);
}

void UI2::Toggle(float x, float y, float w, const std::string& title, bool& val) {
	UI::Label(x, y, 12, title, white());
	val = Engine::Toggle(x + w - 16, y, 16, Icons::checkbox, val, white(), ORIENT_HORIZONTAL);
}

void UI2::Switch(float x, float y, float w, const std::string& title, int c, std::string* nms, int& i) {
	UI::Label(x, y, 12, title, white());
	x += w*sepw;
	float dw = w*sepw2 / c;
	for (int a = 0; a < c; ++a) {
		if (Engine::Button(x + dw * a, y, dw - 1, 16, white(1, (a == i) ? 0.1f : 0.3f), nms[a], 12, white(), true) == MOUSE_RELEASE) {
			i = a;
		}
	}
}

void UI2::Bezier(Vec2 p1, Vec2 t1, Vec2 t2, Vec2 p2, Vec4 col, float thick, int reso) {
	thick /= 2;
	glUseProgram(bezierProg);
	Vec2 pts[4] = { Ds2(p1), Ds2(t1), Ds2(t2), Ds2(p2) };
	glUniform2fv(bezierProgLocs[0], 4, &pts[0][0]);
	glUniform1i(bezierProgLocs[1], reso);
	glUniform2f(bezierProgLocs[2], thick / Display::width, thick / Display::height);
	glUniform4f(bezierProgLocs[3], col.r, col.g, col.b, col.a);
	glBindVertexArray(Camera::emptyVao);
	glDrawArrays(GL_TRIANGLES, 0, reso * 6);
	glUseProgram(0);
}

float UI2::Scroll(float x, float y, float h, float t, float tot, float fill) {
	static float scrpos = -1;

	bool me = Input::mouse0 && (UI::_layer == UI::_layerMax) && Rect(x, y, 8, h).Inside(Input::mouseDownPos);
	UI::Quad(x, y, 8, h, black(0.2f));
	auto scr = Engine::Button(x, y + (h - 2)*t / tot, 8, (h-2)*fill/tot + 2);
	if (scr > 0 || (me && scrpos >= 0)) {
		UI::Quad(x, y + (h - 2)*t / tot, 8, (h - 2)*fill / tot + 2, (scr == MOUSE_HOVER_FLAG) ? white() : white(1, 0.5f));
		if (scr == MOUSE_CLICK) scrpos = Input::mousePos.y - (y + (h - 2)*t / tot);
		else if (scr == MOUSE_RELEASE) scrpos = -1;
		else if (me && scrpos >= 0) {
			t = std::min((Input::mousePos.y - scrpos - y) / (h - 2) * tot, tot - fill);
			t = std::max(t, 0.f);
		}
	}
	else {
		UI::Quad(x + 1, y + 1 + (h - 2)*t / tot, 6, (h - 2)*fill / tot, white(0.5f));
	}
	return t;
}

Vec3 UI2::EditVec(float x, float y, float w, const std::string& t, Vec3 v, bool ena) {
	Vec3 res;
	res.x = TryParse(UI2::EditText(x, y, w, t + " X", std::to_string(v.x), ena, Vec4(0.6f, 0.4f, 0.4f, 1)), 0.f);
	res.y = TryParse(UI2::EditText(x, y + 17, w, t + " Y", std::to_string(v.y), ena, Vec4(0.4f, 0.6f, 0.4f, 1)), 0.f);
	res.z = TryParse(UI2::EditText(x, y + 34, w, t + " Z", std::to_string(v.z), ena, Vec4(0.4f, 0.4f, 0.6f, 1)), 0.f);
	return res;
}