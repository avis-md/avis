#include "ParMenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "vis/pargraphics.h"
#include "utils/dialog.h"
#include "ui/icons.h"
#include "ui/popups.h"
#include "ocl/raytracer.h"

int ParMenu::activeMenu = 0;
const string ParMenu::menuNames[] = { "Particles", "Visualize", "Proteins", "Display", "Raytrace" };
bool ParMenu::expanded = true;
float ParMenu::expandPos = 150;

uint ParMenu::selCnt;
byte ParMenu::drawTypeAll, ParMenu::_drawTypeAll;

void ParMenu::Draw() {
	Engine::DrawQuad(0, 18, expandPos, Display::height - 36.0f, white(0.9f, 0.15f));
	if (expanded) {
		if (!Particles::particleSz) {
			if (Engine::Button(expandPos - 110, Display::height * 0.4f - 40, 80, 80, Icons::openfile, white(0.4f)) == MOUSE_RELEASE) {
				ParLoader::OnOpenFile(Dialog::OpenFile(ParLoader::exts));
			}
			UI::Label(expandPos - 140, Display::height * 0.4f + 62, 12, " Drag & Drop Files here", white());
			UI::Label(expandPos - 140, Display::height * 0.4f + 75, 12, "or Click Button to Import", white());
		}
		else {
			switch (activeMenu) {
			case 0:
				Draw_List();
				break;
			case 2:
				Protein::DrawMenu();
				break;
			case 3:
				ParGraphics::DrawMenu();
				break;
			case 4:
				RayTracer::DrawMenu();
				break;
			}
		}

		for (uint i = 0; i < 5; i++) {
			if (i == activeMenu)
				Engine::DrawQuad(expandPos, 81.0f * i + 18, 17, 81, white(0.9f, 0.15f));
			else
				if (Engine::Button(expandPos, 81.0f * i + 18, 16, 80, white(0.7f, 0.1f), white(1, 0.2f), white(1, 0.05f)) == MOUSE_RELEASE) {
					activeMenu = i;
				}
		}

		Engine::RotateUI(90, Vec2(expandPos + 16, 18));
		UI::font->Align(ALIGN_TOPCENTER);
		for (uint i = 0; i < 5; i++) {
			UI::Label(expandPos + 56 + 81 * i, 18, 12, menuNames[i], white());
		}
		UI::font->Align(ALIGN_TOPLEFT);
		Engine::ResetUIMatrix();

		Engine::DrawQuad(expandPos, Display::height - 34.0f, 16, 16, white(0.9f, 0.15f));
		if ((!UI::editingText && Input::KeyUp(Key_T)) || Engine::Button(expandPos, Display::height - 34.0f, 16, 16, Icons::collapse) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 2.0f, 150.0f);
	}
	else {
		if ((!UI::editingText && Input::KeyUp(Key_T)) || Engine::Button(expandPos, Display::height - 34.0f, 115, 16, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expandPos, Display::height - 34.0f, 16, 16, Icons::expand);
		UI::Label(expandPos + 18, Display::height - 33.0f, 12, "Toolbar (T)", white());
		expandPos = Clamp(expandPos - 1500 * Time::delta, 2.0f, 150.0f);
	}
}

void ParMenu::Draw_List() {
	if (Engine::Button(2, 20, 16, 16, Icons::select, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		SelAll();
	}
	if (Engine::Button(19, 20, 16, 16, Icons::deselect, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		SelClear();
	}
	if (Engine::Button(36, 20, 16, 16, Icons::flipselect, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		SelInv();
	}
	if (!!selCnt) {
		if (Engine::Button(55, 20, 16, 16, Icons::OfDM(drawTypeAll), white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
			Popups::type = POPUP_TYPE::DRAWMODE;
			Popups::pos = Vec2(55, 2);
			_drawTypeAll = drawTypeAll;
			Popups::data = &_drawTypeAll;
		}
		if ((Popups::type == POPUP_TYPE::DRAWMODE) && (_drawTypeAll != drawTypeAll)) {
			drawTypeAll = _drawTypeAll;
			for (uint i = 0; i < Particles::residueListSz; i++) {
				auto& rli = Particles::residueLists[i];
				if (rli.selected) rli.drawType = drawTypeAll;
			}
			ParGraphics::UpdateDrawLists();
		}
	}
	
	Engine::DrawQuad(1, 36, expandPos - 2, Display::height - 37.0f, white(0.9f, 0.1f));
	Engine::BeginStencil(0, 36, expandPos, Display::height - 54.0f);
	float off = 38;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		if (off > 0) {
			Engine::DrawQuad(expandPos - 148, off, 146, 16, rli.selected? Vec4(0.45f, 0.3f, 0.1f, 1) : white(1, 0.3f));
			if (Engine::Button(expandPos - 148, off, 16, 16, rli.expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
				rli.expanded = !rli.expanded;
			}
			UI::Label(expandPos - 132, off, 12, rli.name, white(rli.visible ? 1 : 0.5f));
			if (Engine::Button(expandPos - 130, off, 96, 16) == MOUSE_RELEASE) {
				if (!Input::KeyHold(Key_LeftShift)) {
					if (!(selCnt == 1 && rli.selected)) {
						SelClear();
						drawTypeAll = rli.drawType;
						rli.selected = true;
						selCnt = 1;
					}
					else {
						rli.selected = false;
						selCnt = 0;
					}
				}
				else {
					rli.selected = !rli.selected;
					if (rli.selected && drawTypeAll != rli.drawType) drawTypeAll = 255;
					selCnt += rli.selected ? 1 : -1;
				}
			}
			if (Engine::Button(expandPos - 35, off, 16, 16, Icons::OfDM(rli.drawType), white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
				Popups::type = POPUP_TYPE::DRAWMODE;
				Popups::pos = Vec2(expandPos - 35, off);
				Popups::data = &rli.drawType;
			}
			if (Engine::Button(expandPos - 18, off, 16, 16, rli.visible ? Icons::visible : Icons::hidden) == MOUSE_RELEASE) {
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
					if (Engine::Button(expandPos - 143, off, 16, 16, rj.expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
						rj.expanded = !rj.expanded;
					}
					UI::Label(expandPos - 128, off, 12, rj.name, white((rli.visible && rj.visible) ? 1 : 0.5f));
					if (Engine::Button(expandPos - 35, off, 16, 16, Icons::OfDM(rj.drawType), white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
						Popups::type = POPUP_TYPE::DRAWMODE;
						Popups::pos = Vec2(expandPos - 35, off);
						Popups::data = &rj.drawType;
					}
					if (Engine::Button(expandPos - 18, off, 16, 16, rj.visible ? Icons::visible : Icons::hidden) == MOUSE_RELEASE) {
						rj.visible = !rj.visible;
					}
				}
				off += 17;
				if (off >= Display::height)
					goto loopout;
				if (rj.expanded) {
					auto& sell = ParGraphics::selIds;
					for (uint k = 0; k < rj.cnt; k++) {
						auto itr = std::find(sell.begin(), sell.end(), rj.offset + k + 1);
						bool has = itr != sell.end();
						Engine::DrawQuad(expandPos - 138, off, 136, 16, has? Vec4(0.3f, 0.5f, 0.3f, 1) : white(1, 0.4f));
						UI::Label(expandPos - 136, off, 12, &Particles::particles_Name[(rj.offset + k)*PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, white());
						if (Engine::Button(expandPos - 138, off, 120, 16) == MOUSE_RELEASE) {
							if (!Input::KeyHold(Key_LeftShift)) {
								if (!(sell.size() == 1 && has)) {
									sell.resize(1);
									sell[0] = rj.offset + k + 1;
								}
								else {
									sell.clear();
								}
							}
							else {
								if (has) sell.erase(itr);
								else sell.push_back(rj.offset + k + 1);
							}
						}
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

void ParMenu::SelAll() {
	drawTypeAll = Particles::residueLists[0].drawType;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		rli.selected = true;
		if (drawTypeAll != rli.drawType) drawTypeAll = 255;
	}
	selCnt = Particles::residueListSz;
}

void ParMenu::SelInv() {
	drawTypeAll = Particles::residueLists[0].drawType;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		rli.selected = !rli.selected;
		if (rli.selected && drawTypeAll != rli.drawType) drawTypeAll = 255;
	}
	selCnt = Particles::residueListSz - selCnt;
}

void ParMenu::SelClear() {
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		rli.selected = false;
	}
	selCnt = 0;
}