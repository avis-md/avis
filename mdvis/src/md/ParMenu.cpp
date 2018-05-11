#include "ParMenu.h"
#include "vis/pargraphics.h"
#include "ui/icons.h"

int ParMenu::activeMenu = 0;
const string ParMenu::menuNames[] = { "Particles", "Visualize", "Proteins", "Display" };
bool ParMenu::expanded = true;
float ParMenu::expandPos = 150;
Font* ParMenu::font = nullptr;

void ParMenu::Draw() {
	Engine::DrawQuad(0, 0, expandPos, Display::height - 18.0f, white(0.9f, 0.15f));
	if (expanded) {
		float f = 20;
		switch (activeMenu) {
		case 0:
			Draw_List();
			break;

		case 3:
			Draw_Vis();
			break;
		}

		for (uint i = 0; i < 4; i++) {
			if (i == activeMenu)
				Engine::DrawQuad(expandPos, 81.0f * i, 17, 81, white(0.9f, 0.15f));
			else
				if (Engine::Button(expandPos, 81.0f * i, 16, 80, white(0.7f, 0.1f), white(1, 0.2f), white(1, 0.05f)) == MOUSE_RELEASE) {
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

		Engine::DrawQuad(expandPos, Display::height - 34.0f, 16, 16, white(0.9f, 0.15f));
		if (Engine::Button(expandPos, Display::height - 34.0f, 16, 16, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 2.0f, 150.0f);
	}
	else {
		if (Engine::Button(expandPos, Display::height - 34.0f, 115, 16, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expandPos, Display::height - 34.0f, 16, 16, Icons::expand);
		UI::Label(expandPos + 18, Display::height - 33.0f, 12, "Particle View", font, white());
		expandPos = Clamp(expandPos - 1500 * Time::delta, 2.0f, 150.0f);
	}
}

void ParMenu::Draw_List() {
	if (Engine::Button(2, 2, 16, 16, Icons::select, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {

	}
	if (Engine::Button(19, 2, 16, 16, Icons::deselect, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {

	}
	if (Engine::Button(36, 2, 16, 16, Icons::flipselect, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {

	}
	Engine::DrawQuad(1, 18, expandPos - 2, Display::height - 37.0f, white(0.9f, 0.1f));
	Engine::BeginStencil(0, 0, expandPos, Display::height - 18.0f);
	float off = 20;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		if (off > 0) {
			Engine::DrawQuad(expandPos - 148, off, 146, 16, white(1, 0.3f));
			if (Engine::Button(expandPos - 148, off, 16, 16, rli.expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
				rli.expanded = !rli.expanded;
			}
			UI::Label(expandPos - 132, off, 12, rli.name, font, white(rli.visible ? 1 : 0.5f));
			if (Engine::Button(expandPos - 18, off, 16, 16, rli.visible ? Icons::visible : Icons::hidden, white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
				rli.visible = !rli.visible;
				ParGraphics::UpdateDrawLists();
			}
		}
		off += 17;
		if (off > Display::height)
			goto loopout;
		if (rli.expanded) {
			for (uint j = 0; j < rli.residueSz; j++) {
				auto& rj = rli.residues[j];
				if (off > 0) {
					Engine::DrawQuad(expandPos - 143, off, 141, 16, white(1, 0.35f));
					if (Engine::Button(expandPos - 143, off, 16, 16, rj.expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
						rj.expanded = !rj.expanded;
					}
					UI::Label(expandPos - 128, off, 12, rj.name, font, white((rli.visible && rj.visible) ? 1 : 0.5f));
					if (Engine::Button(expandPos - 18, off, 16, 16, rj.visible ? Icons::visible : Icons::hidden, white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
						rj.visible = !rj.visible;
					}
				}
				off += 17;
				if (off >= Display::height)
					goto loopout;
				if (rj.expanded) {
					for (uint k = 0; k < rj.cnt; k++) {
						Engine::DrawQuad(expandPos - 138, off, 136, 16, white(1, 0.4f));
						UI::Label(expandPos - 136, off, 12, &Particles::particles_Name[(rj.offset + k)*PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, font, white());
						Vec3& col = Particles::colorPallete[Particles::particles_Col[rj.offset + k]];
						Engine::Button(expandPos - 18, off, 16, 16, Icons::circle, Vec4(col, 0.8f), Vec4(col, 1), Vec4(col, 0.5f));
						off += 17;
						if (off >= Display::height)
							goto loopout;
					}
				}
			}
		}
	}
loopout:
	Engine::EndStencil();
}

void ParMenu::Draw_Vis() {
	ParGraphics::DrawMenu();
}