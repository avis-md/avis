#include "ParMenu.h"
#include "ui/icons.h"

int ParMenu::activeMenu = 0;
const string ParMenu::menuNames[] = { "Particles", "Visualize", "Proteins", "Display" };
bool ParMenu::expanded = true;
float ParMenu::expandPos = 150;
Font* ParMenu::font = nullptr;

void ParMenu::Draw() {
	Engine::DrawQuad(0, 0, expandPos, Display::height, white(0.9f, 0.15f));
	if (expanded) {
		float f = 20;
		Engine::BeginStencil(0, 0, expandPos, Display::height);


		Engine::EndStencil();

		for (uint i = 0; i < 4; i++) {
			if (i == activeMenu)
				Engine::DrawQuad(expandPos, 81 * i, 17, 81, white(0.9f, 0.15f));
			else
				if (Engine::Button(expandPos, 81 * i, 16, 80, white(0.7f, 0.1f), white(1, 0.2f), white(1, 0.05f)) == MOUSE_RELEASE) {
					activeMenu = i;
				}
		}

		Engine::RotateUI(90, Vec2(expandPos + 16, 0));
		font->Align(ALIGN_TOPCENTER);
		for (uint i = 0; i < 4; i++) {
			UI::Label(expandPos + 56 + 81 * i, 0, 12, menuNames[i], font, white());
		}
		font->Align(ALIGN_TOPLEFT);
		Engine::ResetUIMatrix();

		Engine::DrawQuad(expandPos, Display::height - 16, 16, 16, white(0.9f, 0.15f));
		if (Engine::Button(expandPos, Display::height - 16, 16, 16, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		Engine::EndStencil();
		expandPos = min(expandPos + 1500 * Time::delta, 150.0f);
	}
	else {
		if (Engine::Button(expandPos, Display::height - 16, 115, 16, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expandPos, Display::height - 16, 16, 16, Icons::expand);
		UI::Label(expandPos + 18, Display::height - 15, 12, "Particle View", font, white());
		expandPos = max(expandPos - 1500 * Time::delta, 0.0f);
	}
}