#include "ui_ext.h"
#include "icons.h"
#include "popups.h"
#include "utils/dialog.h"

string UI2::EditText(float x, float y, uint w, const string& title, const string& val, bool enabled, Vec4 col) {
	UI::Label(x, y, 12, title, white());
	w /= 2;
	if (enabled) {
		return UI::EditText(x + w, y, w - 1, 16, 12, col, val, true, white());
	}
	else {
		Engine::Button(x + w, y, w - 1, 16, col, val, 12, white(0.5f));
		return val;
	}
}

float UI2::Slider(float x, float y, uint w, const string& title, float a, float b, float t) {
	return Slider(x, y, w, title, a, b, t, std::to_string(t));
}

float UI2::Slider(float x, float y, uint w, const string& title, float a, float b, float t, const string& lbl) {
	UI::Label(x, y, 12, title, white());
	w /= 2;
	t = Engine::DrawSliderFill(x + w, y, w - 1, 16, a, b, t, white(1, 0.5f), white());
	UI::Label(x + w + 2, y, 12, lbl, white(1, 0.2f));
	return t;
}

void UI2::Color(float x, float y, uint w, const string& title, Vec4& col) {
	UI::Label(x, y, 12, title, white());
	w /= 2;
	if (Engine::Button(x + w, y, w-1, 16, col) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::COLORPICK;
		Popups::pos = Vec2(x + w, y + 16);
		Popups::data = &col;
	}
	UI::Texture(x + w * 2 - 17, y, 16, 16, Icons::colorwheel);
}

void UI2::File(float x, float y, uint w, const string& title, const string& fl, std::function<void(std::vector<string>)> func) {
	UI::Label(x, y, 12, "File", white());
	w /= 2;
	if (Engine::Button(x + w, y, w-1, 16, white(1, 0.3f), fl, 12, white(0.5f)) == MOUSE_RELEASE) {
		std::vector<string> exts = {"*.hdr"};
		auto res = Dialog::OpenFile(exts);
		if (!!res.size()) {
			func(res);
		}
	}
}

MOUSE_STATUS UI2::Button2(float x, float y, float w, const string& s, Texture* tex, Vec4 col, Vec4 col2) {
	auto ret = Engine::Button(x, y, w, 16, col);
	UI::Texture(x + 1, y, 16, 16, tex, col2);
	UI::font->alignment = ALIGN_TOPCENTER;
	UI::Label(x + 9 + w/2, y, 12, s, col2);
	UI::font->alignment = ALIGN_TOPLEFT;
	return ret;
}

void UI2::Dropdown(float x, float y, float w, const string& title, const Popups::DropdownItem& data) {
	UI::Label(x, y, 12, title, white());
	w /= 2;
	if (Engine::Button(x + w, y, w - 1, 16, white(1, 0.3f), data.list[*data.target], 12, white()) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::DROPDOWN;
		Popups::pos = Vec2(x + w, y + 16);
		Popups::pos2.x = w - 1;
		Popups::data = (Popups::DropdownItem*)&data;
	}
}