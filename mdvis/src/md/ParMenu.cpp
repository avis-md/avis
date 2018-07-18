#include "ParMenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "vis/pargraphics.h"
#include "utils/dialog.h"
#include "ui/icons.h"
#include "ui/popups.h"
#include "ui/ui_ext.h"
#include "ocl/raytracer.h"
#include "mdchan.h"

int ParMenu::activeMenu = 0;
int ParMenu::activeSubMenu[] = {};
const string ParMenu::menuNames[] = { "Particles", "Graphics", "Render", ".", ".." };
bool ParMenu::expanded = true;
float ParMenu::expandPos = 150;
bool ParMenu::showSplash = false;

uint ParMenu::selCnt;
byte ParMenu::drawTypeAll, ParMenu::_drawTypeAll;
bool ParMenu::visibleAll;

std::vector<string> ParMenu::recentFiles, ParMenu::recentFilesN;

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
				if (!!Protein::proCnt) {
					if (UI2::Button2(expandPos - 148, 20, 70, "Atoms", Icons::vis_atom, white(1, 0.3f), (!activeSubMenu[0]) ? accent() : white(0.8f)) == MOUSE_RELEASE) {
						activeSubMenu[0] = 0;
					}
					if (UI2::Button2(expandPos - 77, 20, 75, "Proteins", Icons::vis_prot, white(1, 0.3f), (!activeSubMenu[0]) ? white(0.8f) : accent()) == MOUSE_RELEASE) {
						activeSubMenu[0] = 1;
					}
					if (!activeSubMenu[0]) Draw_List(38);
					else Protein::DrawMenu(38);
				}
				else Draw_List(20);
				break;
			case 1:
				ParGraphics::DrawMenu();
				break;
			case 2:
				RayTracer::DrawMenu();
				break;
			case 3:
			case 4:
				//
				if (Engine::Button(expandPos - 147, 20, 144, 16, white(1, 0.5f), "Toggle grad", 12, white(), true) == MOUSE_RELEASE) {
					ParGraphics::useGradCol = !ParGraphics::useGradCol;
				}
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
			UI::Label(expandPos + 56 + 81 * i, 18, 12, menuNames[i], (i == activeMenu) ? accent() : white());
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

void ParMenu::Draw_List(float off) {
	if (Engine::Button(2, off, 16, 16, Icons::select, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		SelAll();
	}
	if (Engine::Button(19, off, 16, 16, Icons::deselect, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		SelClear();
	}
	if (Engine::Button(36, off, 16, 16, Icons::flipselect, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
		SelInv();
	}
	if (!!selCnt) {
		if (Engine::Button(55, off, 16, 16, Icons::OfDM(drawTypeAll), white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
			Popups::type = POPUP_TYPE::DRAWMODE;
			Popups::pos = Vec2(55, off);
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
		if (Engine::Button(76, off, 16, 16, visibleAll ? Icons::visible : Icons::hidden, white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
			visibleAll = !visibleAll;
			for (uint i = 0; i < Particles::residueListSz; i++) {
				auto& rli = Particles::residueLists[i];
				if (rli.selected) rli.visible = visibleAll;
			}
			ParGraphics::UpdateDrawLists();
		}
	}
	off += 17;
	//Engine::DrawQuad(1, off-1, expandPos - 2, Display::height - 37.0f, white(0.9f, 0.1f));
	Engine::BeginStencil(0, off, expandPos, Display::height - 18 - off);
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		//if (off > 0) {
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
						visibleAll = rli.visible;
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
					visibleAll = rli.visible;
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
		//}
		off += 17;
		if (off > Display::height)
			goto loopout;
		if (rli.expanded) {
			for (uint j = 0; j < rli.residueSz; j++) {
				auto& rj = rli.residues[j];
				//if (off > 0) {
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
				//}
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
									if (Input::dbclick) {
										ParGraphics::rotCenter = Particles::particles_Pos[rj.offset + k];
										Scene::dirty = true;
									}
								}
								else {
									if (Input::dbclick) {
										ParGraphics::rotCenter = Particles::particles_Pos[rj.offset + k];
										Scene::dirty = true;
									}
									else
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

void ParMenu::DrawStart() {
	UI::Texture(0, 0, (float)Display::width, (float)Display::height, ParGraphics::bg, DRAWTEX_CROP);
	MdChan::Draw(Vec2(Display::width * 0.5f, Display::height * 0.3f));
	//UI::Texture(Display::width * 0.5f - Display::height * 0.2f, Display::height * 0.4f, Display::height * 0.4f, Display::height * 0.2f, logo);
	if (ParLoader::busy) {
		Engine::DrawQuad(Display::width * 0.5f - 50, Display::height * 0.6f, 100, 6, white(0.8f, 0.2f));
		Engine::DrawQuad(Display::width * 0.5f - 50, Display::height * 0.6f, 100 * *ParLoader::loadProgress, 6, Vec4(0.9f, 0.7f, 0.2f, 1));
		float oy = 10;
		if (ParLoader::loadProgress2 && *ParLoader::loadProgress2 > 0) {
			Engine::DrawQuad(Display::width * 0.5f - 50, Display::height * 0.6f + 8, 100, 6, white(0.8f, 0.2f));
			Engine::DrawQuad(Display::width * 0.5f - 50, Display::height * 0.6f + 8, 100 * *ParLoader::loadProgress2, 6, Vec4(0.9f, 0.7f, 0.2f, 1));
			oy = 18;
		}
		UI::Label(Display::width * 0.5f - 48, Display::height * 0.6f + oy, 12, ParLoader::loadName);
	}
	else {
		UI::font->Align(ALIGN_TOPCENTER);
		UI::Label(Display::width * 0.5f, Display::height * 0.55f, 12, "Press F1 for Help", white());
		UI::Label(Display::width * 0.5f, Display::height * 0.55f + 14, 12, "Build: " __DATE__ "  " __TIME__, white());
		UI::font->Align(ALIGN_TOPLEFT);
		ParMenu::DrawRecents(Vec4(200, Display::height * 0.55f + 40, Display::width - 400, Display::height * 0.45f - 80));
	}
}

void ParMenu::DrawSplash() {
	UI::IncLayer();
	Engine::DrawQuad(0, 0, Display::width, Display::height, black(0.7f));
	UI::Texture(Display::width*0.5f - 200, Display::height*0.5f - 125, 400, 250, ParGraphics::splash);
	UI::font->Align(ALIGN_TOPRIGHT);
	UI::Label(Display::width * 0.5f + 190, Display::height * 0.5f - 120, 12, __APPVERSION__, white());
	UI::Label(Display::width * 0.5f + 190, Display::height * 0.5f - 104, 12, "Build: " __DATE__ "  " __TIME__, white());
	UI::font->Align(ALIGN_TOPLEFT);
	auto pos = Vec4(Display::width*0.5f - 20, Display::height*0.5f - 80, 215, 183);
	if (!!recentFiles.size()) {
		UI::Label(pos.x + 2, pos.y + 1, 12, "Recent Files", white());
		Engine::DrawQuad(pos.x, pos.y + 17, pos.z, pos.w - 17, white(0.7f, 0.05f));
		for (uint i = 0; i < recentFiles.size(); i++) {
			if (35 + 17 * i > pos.z) break;
			auto ms = Engine::Button(pos.x + 5, pos.y + 20 + 17 * i, pos.z - 10, 16, white(0, 0.4f), recentFilesN[i], 12, white());
			if (ms == MOUSE_RELEASE) {
				showSplash = false;
				ParLoader::OnOpenFile(std::vector<string>{ recentFiles[i] });
			}
			if (ms & MOUSE_HOVER_FLAG) {
				UI::Label(Display::width*0.5f - 190, pos.y + pos.w + 1, 12, recentFiles[i], white(0.7f));
			}
		}
	}
	if ((Input::KeyDown(Key_Escape) && UI::_layer == UI::_layerMax) || (Input::mouse0State == 1 && !Rect(Display::width*0.5f - 200, Display::height*0.5f - 150, 400, 300).Inside(Input::mousePos))) {
		showSplash = false;
	}
}

void ParMenu::SelAll() {
	drawTypeAll = Particles::residueLists[0].drawType;
	visibleAll = false;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		rli.selected = true;
		if (drawTypeAll != rli.drawType) drawTypeAll = 255;
		if (rli.visible) visibleAll = true;
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

void ParMenu::LoadRecents() {
	recentFiles.clear();
	recentFilesN.clear();
	std::ifstream strm(IO::path + "/.recentfiles");
	if (strm.is_open()) {
		string s;
		while (std::getline(strm, s, '\n')) {
			if (!!s.size()) {
				recentFiles.push_back(s);
				recentFilesN.push_back(s.substr(s.find_last_of('/') + 1));
			}
		}
	}
}

void ParMenu::SaveRecents(const string& entry) {
	auto at = std::find(recentFiles.begin(), recentFiles.end(), entry);
	if (at != recentFiles.end()) {
		recentFiles.erase(at);
		int loc = at - recentFiles.begin();
		recentFilesN.erase(recentFilesN.begin() + loc);
	}
	else if (recentFiles.size() >= 10) {
		recentFiles.pop_back();
		recentFilesN.pop_back();
	}
	recentFiles.insert(recentFiles.begin(), entry);
	recentFilesN.insert(recentFilesN.begin(), entry.substr(entry.find_last_of('/') + 1));
	std::ofstream strm(IO::path + "/.recentfiles");
	for (auto& s : recentFiles) {
		strm << s << "\n";
	}
}

void ParMenu::DrawRecents(Vec4 pos) {
	if (!recentFiles.size()) return;
	UI::Label(pos.x + 2, pos.y + 1, 12, "Recent Files", white());
	Engine::DrawQuad(pos.x, pos.y + 17, pos.z, pos.w - 17, white(0.7f, 0.05f));
	for (uint i = 0; i < recentFiles.size(); i++) {
		if (35 + 17 * i > pos.w) break;
		if (Engine::Button(pos.x + 5, pos.y + 20 + 17 * i, pos.z - 10, 16, white(0, 0.4f), recentFilesN[i], 12, white()) == MOUSE_RELEASE) {
			ParLoader::OnOpenFile(std::vector<string>{ recentFiles[i] });
		}
		UI::Label(pos.x + pos.z * 0.3f, pos.y + 20 + 17 * i, 12, recentFiles[i], white(0.5f), pos.z * 0.7f - 5);
	}
}