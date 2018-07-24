#include "popups.h"
#include "vis/pargraphics.h"
#include "md/ParMenu.h"

POPUP_TYPE Popups::type = POPUP_TYPE::NONE;
Vec2 Popups::pos, Popups::pos2;
void* Popups::data = 0;

void Popups::Draw() {
    if (type == POPUP_TYPE::NONE) return;
    UI::IncLayer();

	if (Input::KeyDown(Key_Escape)) {
		type = POPUP_TYPE::NONE;
		return;
	}

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
	case POPUP_TYPE::DROPDOWN:
		DrawDropdown();
		break;
	case POPUP_TYPE::RESNM:
	case POPUP_TYPE::RESID:
	case POPUP_TYPE::ATOMID:
		ParMenu::DrawSelPopup();
		break;
	case POPUP_TYPE::SYSMSG:
		VisSystem::DrawMsgPopup();
		break;
    default:
		Debug::Warning("Popup", "case not handled: " + std::to_string((int)type) + "!");
		type = POPUP_TYPE::NONE;
        break;
    }
}

void Popups::DrawMenu() {
	auto mn = (std::vector<MenuItem>*)data;
	if (DoDrawMenu(mn, pos.x, pos.y)) {
		type = POPUP_TYPE::NONE;
	}
}

void Popups::DrawDropdown() {
	auto dt = (DropdownItem*)data;
	uint n = 0;
	while (!!dt->list[n][0]) n++;
	Engine::DrawQuad(pos.x-1, pos.y, pos2.x+2, 16*n + 1, black(0.7f));
	for (uint a = 0; a < n; a++) {
		if (Engine::Button(pos.x, pos.y + 16*a, pos2.x, 16, white(1, 0.2f), dt->list[a], 12, white()) == MOUSE_RELEASE) {
			(*dt->target) = a;
			Popups::type = POPUP_TYPE::NONE;
		}
	}
	if ((Input::mouse0State == 1) && !Engine::Button(pos.x, pos.y, pos2.x, 16*n)) {
		Popups::type = POPUP_TYPE::NONE;
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
			type = POPUP_TYPE::NONE;
			if (i.callback) i.callback();
		}
	}
	bool clk = (Input::mouse0State == 1) && !Engine::Button(x - 1, y, 122, 18 * sz + 3);
	bool hvr = !Rect(x - 30, y - 30, 182, 18 * sz + 3 + 60).Inside(Input::mousePos);
	return (clk || hvr);
}