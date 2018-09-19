#include "ParMenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "vis/pargraphics.h"
#include "utils/dialog.h"
#include "ui/localizer.h"
#include "ui/icons.h"
#include "ui/popups.h"
#include "ui/ui_ext.h"
#include "ocl/raytracer.h"

int ParMenu::activeMenu = 0;
int ParMenu::activeSubMenu[] = {};
string ParMenu::menuNames[];
bool ParMenu::expanded = true;
float ParMenu::expandPos = 150;
bool ParMenu::showSplash = true;

uint ParMenu::selCnt;
byte ParMenu::drawTypeAll, ParMenu::_drawTypeAll;
bool ParMenu::visibleAll;

std::vector<string> ParMenu::recentFiles, ParMenu::recentFilesN;

float _off = 0;

void ParMenu::Init() {
	menuNames[0] = _("Particles");
	menuNames[1] = _("Colors");
	menuNames[2] = _("Graphics");
	menuNames[3] = _("Render");
	menuNames[4] = "Information";
}

void ParMenu::Draw() {
	UI::Quad(0, 18, expandPos, Display::height - 36.0f, white(0.9f, 0.15f));
	if (expanded) {
		if (!Particles::particleSz) {
			if (Engine::Button(expandPos - 110, Display::height * 0.4f - 40, 80, 80, Icons::openfile, white(0.4f)) == MOUSE_RELEASE) {
				ParLoader::OnOpenFile(Dialog::OpenFile(ParLoader::exts));
			}
			UI::Label(expandPos - 140, Display::height * 0.4f + 62, 12, _(" Drag & Drop Files here"), white());
			UI::Label(expandPos - 140, Display::height * 0.4f + 75, 12, _("or Click Button to Import"), white());
		}
		else {
			switch (activeMenu) {
			case 0:
				if (!!Protein::proCnt) {
					if (UI2::Button2(expandPos - 148, 20, 70, _("Atoms"), Icons::vis_atom, white(1, 0.3f), (!activeSubMenu[0]) ? VisSystem::accentColor : white(0.8f)) == MOUSE_RELEASE) {
						activeSubMenu[0] = 0;
					}
					if (UI2::Button2(expandPos - 77, 20, 75, _("Proteins"), Icons::vis_prot, white(1, 0.3f), (!activeSubMenu[0]) ? white(0.8f) : VisSystem::accentColor) == MOUSE_RELEASE) {
						activeSubMenu[0] = 1;
					}
					if (!activeSubMenu[0]) Draw_List(38);
					else Protein::DrawMenu(38);
				}
				else Draw_List(20);
				break;
			case 1:
				ParGraphics::DrawColMenu();
				break;
			case 2:
				ParGraphics::DrawMenu();
				break;
			case 3:
				RayTracer::DrawMenu();
				break;
			case 4:
				//
				UI::Label(expandPos - 148, 30, 12, "Particle count: " + std::to_string(Particles::particleSz), white());
				break;
			}
		}

		for (uint i = 0; i < 5; i++) {
			if (i == activeMenu)
				UI::Quad(expandPos, 81.0f * i + 18, 17, 81, white(0.9f, 0.15f));
			else
				if (Engine::Button(expandPos, 81.0f * i + 18, 16, 80, white(0.7f, 0.1f), white(1, 0.2f), white(1, 0.05f)) == MOUSE_RELEASE) {
					activeMenu = i;
				}
		}

		Engine::RotateUI(90, Vec2(expandPos + 16, 18));
		UI::font->Align(ALIGN_TOPCENTER);
		for (uint i = 0; i < 5; i++) {
			UI::Label(expandPos + 56 + 81 * i, 18, 12, menuNames[i], (i == activeMenu) ? VisSystem::accentColor : white());
		}
		UI::font->Align(ALIGN_TOPLEFT);
		Engine::ResetUIMatrix();

		UI::Quad(expandPos, Display::height - 34.0f, 16, 16, white(0.9f, 0.15f));
		if ((!UI::editingText && Input::KeyUp(Key_T)) || Engine::Button(expandPos, Display::height - 34.0f, 16, 16, Icons::collapse) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 2.0f, 150.0f);
	}
	else {
		if ((!UI::editingText && Input::KeyUp(Key_T)) || Engine::Button(expandPos, Display::height - 34.0f, 115, 16, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expandPos, Display::height - 34.0f, 16, 16, Icons::expand);
		UI::Label(expandPos + 18, Display::height - 33.0f, 12, _("Toolbar (T)"), white());
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
		if (Engine::Button(110, off, 16, 16, Icons::down) == MOUSE_RELEASE) {
			Particles::particles_Conn.visible = !Particles::particles_Conn.visible;
		}
	}
	else {
		Particles::particles_Conn.visible = false;
	}
	off += 17;
	if (!!selCnt && Particles::particles_Conn.visible) {
		DrawConnMenu(Particles::particles_Conn, 1, off, 148);
	}
	//UI::Quad(1, off-1, expandPos - 2, Display::height - 37.0f, white(0.9f, 0.1f));
	Engine::BeginStencil(0, off, expandPos, Display::height - 18 - off);
	if (Rect(0, off, expandPos, Display::height - 18 - off).Inside(Input::mousePos)) {
		_off -= Input::mouseScroll * 20;
		_off = min(_off, 0.0f);
	}
	off -= _off;
	float mof = off;
	static byte drawTypeOld;
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		//if (off > 0) {
			UI::Quad(expandPos - 148, off, 146, 16, rli.selected? Vec4(0.45f, 0.3f, 0.1f, 1) : white(1, 0.3f));
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
				drawTypeOld = rli.drawType;
				Popups::data = &rli.drawType;
			}
			else if (Popups::type == POPUP_TYPE::DRAWMODE && Popups::data == &rli.drawType && drawTypeOld != rli.drawType) {
				drawTypeOld = rli.drawType;
				for (uint n = 0; n < rli.residueSz; n++) {
					rli.residues[n].drawType = rli.drawType;
				}
				ParGraphics::UpdateDrawLists();
			}
			if (Engine::Button(expandPos - 18, off, 16, 16, rli.visible ? Icons::visible : Icons::hidden) == MOUSE_RELEASE) {
				rli.visible = !rli.visible;
				rli.visibleAll = true;
				for (uint n = 0; n < rli.residueSz; n++) {
					rli.residues[n].drawType = rli.visible;
				}
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
					UI::Quad(expandPos - 143, off, 141, 16, white(1, 0.35f));
					if (Engine::Button(expandPos - 143, off, 16, 16, rj.expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
						rj.expanded = !rj.expanded;
					}
					UI::Label(expandPos - 128, off, 12, rj.name, white((rli.visible && rj.visible) ? 1 : 0.5f));
					if (Engine::Button(expandPos - 35, off, 16, 16, Icons::OfDM(rj.drawType), white(0.8f), white(), white(1, 0.7f)) == MOUSE_RELEASE) {
						Popups::type = POPUP_TYPE::DRAWMODE;
						Popups::pos = Vec2(expandPos - 35, off);
						drawTypeOld = rj.drawType;
						Popups::data = &rj.drawType;
					}
					else if (Popups::type == POPUP_TYPE::DRAWMODE && Popups::data == &rj.drawType && drawTypeOld != rj.drawType) {
						drawTypeOld = rj.drawType;
						rli.drawType = 255;
						ParGraphics::UpdateDrawLists();
					}
					if (Engine::Button(expandPos - 18, off, 16, 16, rj.visible ? Icons::visible : Icons::hidden) == MOUSE_RELEASE) {
						rj.visible = !rj.visible;
						rli.visibleAll = false;
						ParGraphics::UpdateDrawLists();
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
						UI::Quad(expandPos - 138, off, 136, 16, has? Vec4(0.3f, 0.5f, 0.3f, 1) : white(1, 0.4f));
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
	if (ParLoader::busy) {
		UI::Quad(Display::width * 0.5f - 50, Display::height * 0.6f, 100, 6, white(0.8f, 0.2f));
		UI::Quad(Display::width * 0.5f - 50, Display::height * 0.6f, 100 * *ParLoader::loadProgress, 6, Vec4(0.9f, 0.7f, 0.2f, 1));
		float oy = 10;
		if (ParLoader::loadProgress2 && *ParLoader::loadProgress2 > 0) {
			UI::Quad(Display::width * 0.5f - 50, Display::height * 0.6f + 8, 100, 6, white(0.8f, 0.2f));
			UI::Quad(Display::width * 0.5f - 50, Display::height * 0.6f + 8, 100 * *ParLoader::loadProgress2, 6, Vec4(0.9f, 0.7f, 0.2f, 1));
			oy += 14;
			UI::Label(Display::width * 0.5f - 48, Display::height * 0.6f + oy + 16, 12, "Frame " + std::to_string(*ParLoader::loadFrames));
		}
		UI::Label(Display::width * 0.5f - 48, Display::height * 0.6f + oy, 12, ParLoader::loadName);
	}
}

void ParMenu::DrawSplash() {
	UI::IncLayer();
	UI::Quad(0, 0, (float)Display::width, (float)Display::height, black(0.7f));
	UI::Texture(Display::width*0.5f - 200, Display::height*0.5f - 125, 400, 250, ParGraphics::splash);
	UI::font->Align(ALIGN_TOPRIGHT);
	UI::Label(Display::width * 0.5f + 190, Display::height * 0.5f - 120, 12, VERSIONSTRING, white());
	UI::Label(Display::width * 0.5f + 190, Display::height * 0.5f - 104, 12, "Build hash: " + VisSystem::version_hash, white());
	UI::font->Align(ALIGN_TOPLEFT);

	string sub = "";

	auto ms = Engine::Button(Display::width*0.5f - 180, Display::height*0.5f - 63, 140, 54, white(0.3f), _("User Manual"), 12, white(), true);
	if (ms & MOUSE_HOVER_FLAG) {
		sub = "Displays the user manual in the browser";
		if (ms == MOUSE_RELEASE) {
			IO::OpenEx(IO::path + "docs/index.html");
		}
	}
	ms = Engine::Button(Display::width*0.5f - 180, Display::height*0.5f - 63 + 56, 140, 54, white(0.3f), _("Report A Problem"), 12, white(), true);
	if (ms & MOUSE_HOVER_FLAG) {
		sub = "Opens the problem reporter form (Google Forms)";
		if (ms == MOUSE_RELEASE) {
			IO::OpenEx("https://goo.gl/forms/U1hhBUHuo2CyiDEn1");
		}
	}
	ms = Engine::Button(Display::width*0.5f - 180, Display::height*0.5f - 63 + 112, 140, 54, white(0.3f), _("Request A Feature"), 12, white(), true);
	if (ms & MOUSE_HOVER_FLAG) {
		sub = "Opens the feature request form (Google Forms)";
		if (ms == MOUSE_RELEASE) {
			IO::OpenEx("https://goo.gl/forms/2SUhPlWFRBQuSg4D2");
		}
	}

	auto pos = Vec4(Display::width*0.5f - 20, Display::height*0.5f - 80, 215, 183);
	if (!!recentFiles.size()) {
		UI::Label(pos.x + 2, pos.y + 1, 12, _("Recent Files"), white());
		UI::Quad(pos.x, pos.y + 17, pos.z, pos.w - 17, white(0.7f, 0.05f));
		for (uint i = 0; i < recentFiles.size(); i++) {
			if (35 + 17 * i > pos.w) break;
			ms = Engine::Button(pos.x + 5, pos.y + 20 + 17 * i, pos.z - 10, 16, white(0, 0.4f), recentFilesN[i], 12, white());
			if (ms & MOUSE_HOVER_FLAG) {
				sub = recentFiles[i];
				if (Engine::Button(pos.x + pos.z - 21, pos.y + 20 + 17 * i, 16, 16, Icons::cross, red()) == MOUSE_RELEASE) {
					RemoveRecent(i);
					break;
				}
				else if (ms == MOUSE_RELEASE) {
					if (!IO::HasFile(recentFiles[i])) {
						RemoveRecent(i);
						VisSystem::SetMsg("File does not exist!", 1);
						break;
					}
					showSplash = false;
					ParLoader::OnOpenFile(std::vector<string>{ recentFiles[i] });
				}
			}
		}
	}
	else UI::Label(pos.x + 2, pos.y + 1, 12, _("No Recent Files"), white());
	if (Engine::Button(pos.x + pos.z - 70, pos.y, 70, 16, white(1, 0.3f), _("Browse"), 12, white(), true) == MOUSE_RELEASE) {
		ParLoader::OnOpenFile(Dialog::OpenFile(ParLoader::exts));
	}
	if (sub.size()) {
		UI::Label(Display::width*0.5f - 190, pos.y + pos.w + 1, 12, sub, white(0.7f));
	}
 	if ((Input::KeyDown(Key_Escape) && UI::_layer == UI::_layerMax) || (Input::mouse0State == 1 && !Rect(Display::width*0.5f - 200, Display::height*0.5f - 150, 400, 300).Inside(Input::mousePos))) {
		showSplash = false;
	}
}

void ParMenu::DrawConnMenu(Particles::conninfo& cn, float x, float& off, float width) {
#define SV(v) auto _ ## v = cn.v
#define CP(v) if (_ ## v != cn.v) { cn.v = _ ## v; Scene::dirty = true; }
	float sz = 17 * (((!cn.drawMode)? (cn.dashed? 5 : 3) : 2) + (cn.usecol? 2 : 1)) + 2.0f;
	UI::Quad(x, off, width, sz, white(0.7f, 0.15f));
	off++;
	bool dl = !!cn.drawMode;
	UI2::Toggle(x + 2, off, width - 4, "Solid", dl);
	if (dl != !!cn.drawMode) {
		cn.drawMode = dl? 1 : 0;
		Scene::dirty = true;
	}
	off += 17;
	if (dl) {
		auto _scale = UI2::Slider(x + 2, off, (uint)width - 4, "Thickness", 0, 1, cn.scale);
		off += 17;
		CP(scale);
	}
	else {
		SV(dashed);
		auto _line_sc = UI2::Slider(x + 2, off, (uint)width - 4, "Thickness", 1, 5, cn.line_sc);
		off += 17;
		UI2::Toggle(x + 2, off, width - 4, "Dashed lines", _dashed);
		off += 17;
		if (_dashed) {
			auto _line_sp = UI2::Slider(x + 2, off, (uint)width - 4, "Spacing", 0, 1, cn.line_sp);
			off += 17;
			auto _line_rt = UI2::Slider(x + 2, off, (uint)width - 4, "Ratio", 0, 1, cn.line_rt);
			off += 17;
			CP(line_sp); CP(line_rt);
		}
		CP(dashed); CP(line_sc);
	}
	SV(usecol);
	UI2::Toggle(x + 2, off, width - 4, "Custom Colors", _usecol);
	off += 17;
	if (_usecol) {
		SV(col);
		UI2::Color(x + 2, off, (uint)width - 4, "Color", cn.col);
		off += 18;
		if (_col != cn.col) Scene::dirty = true;
	}
	else off++;
	CP(usecol);
#undef SV
#undef CP
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

void ParMenu::DrawSelPopup() {
	UI::Quad(0, 20, 150, Display::height - 40.0f, white(1, 0.1f));
	UI::Label(2, 20, 12, "Select", white());
	if (Engine::Button(100, 20, 49, 16, white(1, 0.4f), "Cancel", 12, white(), true) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::NONE;
		return;
	}
	float off = 38;
	Engine::BeginStencil(0, off, expandPos, Display::height - 18 - off);
	for (uint i = 0; i < Particles::residueListSz; i++) {
		auto& rli = Particles::residueLists[i];
		UI::Quad(expandPos - 148, off, 146, 16, white(1, 0.3f));
		//UI::Label(expandPos - 132, off, 12, rli.name, white());
		if (Engine::Button(expandPos - 130, off, 128, 16, white(0), rli.name, 12, white()) == MOUSE_RELEASE && Popups::type == POPUP_TYPE::RESNM) {
			*(string*)Popups::data = string(rli.name);
			Popups::type = POPUP_TYPE::NONE;
		}
		if (Popups::type == POPUP_TYPE::RESID || Popups::type == POPUP_TYPE::ATOMID) {
			if (Engine::Button(expandPos - 148, off, 16, 16, rli.expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
				rli.expanded = !rli.expanded;
			}
			off += 17;
			if (off > Display::height)
				goto loopout;
			if (rli.expanded) {
				for (uint j = 0; j < rli.residueSz; j++) {
					auto& rj = rli.residues[j];
					UI::Quad(expandPos - 143, off, 141, 16, white(1, 0.35f));
					if (Engine::Button(expandPos - 126, off, 124, 16, white(0), rj.name, 12, white()) == MOUSE_RELEASE && Popups::type == POPUP_TYPE::RESID) {
						//*(uint*)Popups::data = ;
						Popups::type = POPUP_TYPE::NONE;
					}
					if (Popups::type == POPUP_TYPE::ATOMID) {
						if (Engine::Button(expandPos - 143, off, 16, 16, rj.expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
							rj.expanded = !rj.expanded;
						}
						off += 17;
						if (off >= Display::height)
							goto loopout;
						if (rj.expanded) {
							for (uint k = 0; k < rj.cnt; k++) {
								UI::Quad(expandPos - 138, off, 136, 16, white(1, 0.4f));
								UI::Label(expandPos - 136, off, 12, &Particles::particles_Name[(rj.offset + k)*PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, white());
								if (Engine::Button(expandPos - 138, off, 120, 16) == MOUSE_RELEASE) {
									*(uint*)Popups::data = rj.offset + k;
									Popups::type = POPUP_TYPE::NONE;
								}
								off += 17;
								if (off >= Display::height)
									goto loopout;
							}
						}
					}
					else off += 17;
				}
			}
		}
		else off += 17;
	}
loopout:
	Engine::EndStencil();
}

void ParMenu::LoadRecents() {
	recentFiles.clear();
	recentFilesN.clear();
	std::ifstream strm(IO::path + ".recentfiles");
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
	std::ofstream strm(IO::path + ".recentfiles");
	for (auto& s : recentFiles) {
		strm << s << "\n";
	}
}

void ParMenu::RemoveRecent(uint i) {
	recentFiles.erase(recentFiles.begin() + i);
	recentFilesN.erase(recentFilesN.begin() + i);
	std::ofstream strm(IO::path + ".recentfiles");
	for (auto& s : recentFiles) {
		strm << s << "\n";
	}
}