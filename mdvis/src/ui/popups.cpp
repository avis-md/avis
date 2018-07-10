#include "popups.h"
#include "vis/pargraphics.h"

POPUP_TYPE Popups::type = POPUP_TYPE::NONE;
Vec2 Popups::pos = Vec2();
void* Popups::data = 0;

void Popups::Draw() {
    if (type == POPUP_TYPE::NONE) return;
    UI::IncLayer();

    switch (type) {
	case POPUP_TYPE::MENU:
		DrawMenu();
		break;
    case POPUP_TYPE::DRAWMODE:
        ParGraphics::DrawPopupDM();
        break;
	case POPUP_TYPE::COLORPICK:
		Color::DrawPicker();
		break;
    default:
        break;
    }
}

void Popups::DrawMenu() {
	auto mn = (std::vector<MenuItem>*)data;
	if (DoDrawMenu(mn, pos.x, pos.y)) {
		type = POPUP_TYPE::NONE;
	}
}

bool Popups::DoDrawMenu(std::vector<MenuItem>* mn, float x, float y) {
	auto sz = mn->size();
	Engine::DrawQuad(x - 1, y, 122, 18 * sz + 1, black(0.7f));
	Engine::DrawQuad(x, y, 120, 18 * sz, white(1, 0.1f));
	for (size_t a = 0; a < sz; a++) {
		auto& i = mn->at(a);
		auto st = Engine::Button(x + 1, y + 18 * a + 1, 118, 16, white(0), white(1, 0.2f), white(1, 0.05f));
		Engine::DrawQuad(x + 2, y + 18 * a + 1, 16, 16, i.icon);
		UI::Label(x + 22, y + 18 * a + 1, 12, i.label, white((!st) ? 0.7f : 1));
		if ((st | MOUSE_HOVER_FLAG) && i.child.size()) {
			
		}
		if (st == MOUSE_RELEASE) {
			if (i.callback) i.callback();
			type = POPUP_TYPE::NONE;
		}
	}
	bool clk = (Input::mouse0State == 1) && !Engine::Button(x - 1, y, 122, 18 * sz + 3);
	bool hvr = !Rect(x - 30, y - 30, 182, 18 * sz + 3 + 60).Inside(Input::mousePos);
	return (clk || hvr);
}