#include "system.h"
#include "py/PyWeb.h"
#include "ui/icons.h"

Vec4 VisSystem::accentColor = Vec4(1, 1, 1, 1);
uint VisSystem::renderMs, VisSystem::uiMs;
Font* VisSystem::font;

VIS_MOUSE_MODE VisSystem::mouseMode = VIS_MOUSE_MODE::ROTATE;

bool VisSystem::InMainWin(const Vec2& pos) {
	if (PyWeb::drawFull) return false;
	else return (pos.x > ParMenu::expandPos + 16) && (pos.x < Display::width - PyWeb::expandPos) && (pos.y < Display::height - 18);
}

void VisSystem::DrawBar() {
	Engine::DrawQuad(0, Display::height - 18.0f, (float)Display::width, 18, white(0.9f, 0.1f));
	UI::Label(2, Display::height - 16.0f, 12, "Render: " + std::to_string(renderMs) + "ms  UI: " + std::to_string(uiMs) + "ms", font, white(0.5f));

	byte sel = (byte)mouseMode;
	for (byte b = 0; b < 3; b++) {
		if (Engine::Button(170 + 17 * b, Display::height - 17.0f, 16, 16, (&Icons::toolRot)[b], (sel == b) ? Vec4(1, 0.7f, 0.4f, 1) : white(0.7f), white(), white(0.5f)) == MOUSE_RELEASE) {
			mouseMode = (VIS_MOUSE_MODE)b;
		}
	}
}