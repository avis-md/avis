#include "help.h"

Font* HelpMenu::font = nullptr;

bool HelpMenu::show = false;
float HelpMenu::alpha = 0;

void HelpMenu::Draw() {
	UI::IncLayer();
	if (!UI::editingText) {
		if (!show && Input::KeyDown(Key_F1)) show = true;
		if (show && Input::KeyDown(Key_Escape)) show = false;
	}
	if (alpha > 0) {
		UI::alpha = alpha;

		Engine::DrawQuad(0, 0, Display::width, Display::height, black(0.95f));
		UI::Label(10, 5, 14, "HELP", font, white());
		font->Align(ALIGN_TOPRIGHT);
		UI::Label(Display::width - 5, 5, 10, "version 0.01", font, white(0.7f));
		font->Align(ALIGN_TOPLEFT);

		UI::Label(10, Display::height - 16, 12, "This program is under development. Please submit any bugs / suggestions to puakai95@keio.jp", font, white(0.7f));
		UI::alpha = 1;
	}
	alpha = Clamp(alpha + (show? 1 : -1)*5*Time::delta, 0.0f, 1.0f);
}