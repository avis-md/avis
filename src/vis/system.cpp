// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#include "system.h"
#include "pargraphics.h"
#include "renderer.h"
#include "changelog.h"
#include "preferences.h"
#include "md/parmenu.h"
#include "imp/parloader.h"
#include "web/anweb.h"
#include "ui/icons.h"
#include "ui/help.h"
#include "ui/ui_ext.h"
#include "ui/localizer.h"
#include "ui/browse.h"
#include "res/resdata.h"
#include "live/livesyncer.h"
#include "utils/dialog.h"
#include "utils/xml.h"
#include "utils/runcmd.h"
#include "utils/effects.h"
#include "utils/tinyfiledialogs.h"

#ifdef PLATFORM_WIN
#define EXPPATH "path=%path%;\"" + IO::path + "\"&&"
#else
#define EXPPATH
#endif

#define LBD_IMPORT\
	const auto fs = Dialog::OpenFile(ParLoader::exts);\
	if (!!fs.size()) {\
		ParLoader::isSrv = false;\
		ParLoader::OnOpenFile(fs);\
	}

std::string VisSystem::version_hash =
#include "../githash.h"
;

std::string VisSystem::localFd = "";

Vec4 VisSystem::accentColor = Vec4(1, 0.75f, 0, 1), VisSystem::backColor = white(1, 0.1f);
bool VisSystem::blur = true;
float VisSystem::blurRad = 10;
float VisSystem::opacity = 0.7f;
uint VisSystem::renderMs, VisSystem::uiMs;

float VisSystem::lastSave;
std::string VisSystem::currentSavePath, VisSystem::currentSavePath2;

std::vector<Popups::MenuItem> VisSystem::menuItems[];

std::string VisSystem::message;
std::string VisSystem::message2 = "";
bool VisSystem::hasMessage2 = false;
byte VisSystem::messageSev = 0;

void VisSystem::SetMsg(const std::string& msg, byte sev, const std::string& msg2) {
	message = msg;
	message2 = msg2;
	hasMessage2 = !!msg2.size();
	messageSev = sev;

	if (hasMessage2 && sev == 2) {
		Popups::type = POPUP_TYPE::SYSMSG;
	}
	Debug::Message("VisSystem", "message set to \"" + msg + "\"");
}

VIS_MOUSE_MODE VisSystem::mouseMode = VIS_MOUSE_MODE::ROTATE;

float VisSystem::_defBondLength = 0.0225f; // 0.15
std::unordered_map<uint, float> VisSystem::_bondLengths;
std::unordered_map<ushort, float> VisSystem::radii;

void VisSystem::Init() {
	message = _("Hello!");
	radii.clear();

	auto& mi = menuItems[0];
	mi.resize(7);
	mi[0].Set(Icons::newfile, _("New"), []() {
		Particles::Clear();
		AnWeb::Clear0();
	});
	mi[1].Set(Icons::openfile, _("Restore"), []() {
		auto res = Dialog::OpenFile({ "*" EXT_SVFL });
		if (!!res.size()) {
			VisSystem::Load(res[0].substr(0, res[0].find_last_of('.')));
		}
	});
	mi[2].Set(0, _("Restore..."), 0);
	mi[3].Set(0, _("Save"), []() {
		auto path = Dialog::SaveFile(EXT_SVFL);
		if (!!path.size()) VisSystem::Save(path.substr(0, path.size() - strlen_c(EXT_SVFL)));
	});
	mi[4].Set(Icons::openfile, _("Import"), []() {
		LBD_IMPORT
	});
	mi[5].Set(Icons::openfile, _("Import Recent"), 0);
	mi[6].Set(Icons::openfile, _("Import Remote"), []() {
		ParLoader::isSrv = true;
		Browse::mode = Browse::MODE::OPENFILE;
		Browse::callback = [](std::string s) {
			auto cc = s.c_str();
			ParLoader::OnDropFile(1, &cc);
		};
		if (ParLoader::srv.ok) Browse::system = new RemoteBrowseTarget();
	});

	auto& mi0 = menuItems[1];
	mi0.resize(1);
	mi0[0].Set(0, _("Nodes folder"), []() {
		IO::OpenFd(AnWeb::nodesPath);
	});

	auto& mi1 = menuItems[2];
	mi1.resize(2);
	mi1[0].Set(0, "Autorotate", []() {
		ParGraphics::autoRot = true;
	});
	mi1[1].Set(0, "Preferences", []() {
		Preferences::show = true;
	});

	auto& mi2 = menuItems[3];
	mi2.resize(3);
	mi2[0].Set(0, "Image (GLSL)", []() {
		VisRenderer::ToImage();
	});
	//mi2[1].Set(0, "Image (Raytraced)", 0);
	mi2[1].Set(0, "Movie (GLSL)", []() {
		VisRenderer::ToVid();
	});
	//mi2[3].Set(0, "Movie (Raytraced)", 0);
	mi2[2].Set(0, "Options", []() {
		ParMenu::expanded = true;
		ParMenu::activeMenu = 3;
	});

	auto& mi3 = menuItems[4];
	mi3.resize(3);
	mi3[0].Set(0, "User Manual", []() {
		IO::OpenEx(IO::path + "docs/index.html");
	});
	mi3[1].Set(0, "Change Log", []() {
		ChangeLog::show = true;
	});
	mi3[2].Set(Icons::vis_atom, "Splash Screen", []() {
		ParMenu::showSplash = true;
	});

	Preferences::Link("SHQUI", &blur);
	Preferences::Link("SUIBL", &blurRad, []() {
		ParGraphics::tfboDirty = true;
	});
	Preferences::Link("SOPUI", &opacity);
	Preferences::Link("SACCL", &accentColor, []() {
		accentColor.a = 1;
	});
	Preferences::Link("SBKCL", &backColor, []() {
		backColor.a = 1;
	});

	Preferences::Link("VSS", &Input::scrollScl);
}

void VisSystem::InitEnv() {
	localFd = IO::userPath + ".avis/";
	if (!IO::HasDirectory(localFd))
		IO::MakeDirectory(localFd);
}

bool VisSystem::InMainWin(const Vec2& pos) {
	if (AnWeb::drawFull) return false;
	else return (pos.x > ParMenu::expandPos + 16) && (pos.x < Display::width - ((Particles::empty || LiveSyncer::activeRunner)? LiveSyncer::expandPos : AnWeb::expandPos)) && (pos.y > 18) && (pos.y < Display::height - 18);
}

void VisSystem::UpdateTitle() {
	auto& c = menuItems[0][5].child;
	auto s = ParMenu::recentFiles.size();
	c.resize(s);
	for (size_t i = 0; i < s; ++i) {
		c[i].Set(0, ParMenu::recentFilesN[i], []() {
			const char* cc[1] = { ParMenu::recentFiles[Popups::selectedMenu].c_str() };
			ParLoader::OnDropFile(1, cc);
			Popups::type = POPUP_TYPE::NONE;
		});
		c[i].icon = GLuint(-1);
	}
}

void VisSystem::DrawTitle() {
	UI2::BackQuad(0, 0, (float)Display::width, 18);
	UI::Quad(0, 0, (float)Display::width, 18, black(0.5f));
	const std::string menu[] = {_("File"), _("Edit"), _("Options"), _("Render"), _("Help")};
	bool iso = Popups::type == POPUP_TYPE::MENU && Popups::data >= menuItems && Popups::data < (menuItems + 5) && UI::_layerMax == UI::_layer+1;
	UI::ignoreLayers = iso; 
	for (uint i = 0; i < 5; ++i) {
		auto st = Engine::Button(2.f + 60 * i, 1, 59, 16, white(0), menu[i], 12, white(), true);
		if (st == MOUSE_RELEASE || (!!(st & MOUSE_HOVER_FLAG) && iso)) {
			Popups::type = POPUP_TYPE::MENU;
			Popups::pos = Vec2(2 + 60 * i, 17);
			Popups::data = menuItems + i;
		}
	}
	UI::ignoreLayers = false;
	UI::Quad(Display::width * 0.6f, 1, Display::width * 0.3f, 16, white(0.2f));
	UI::Label(Display::width * 0.6f + 2, 1, 12, message, (!messageSev) ? white(0.5f) : ((messageSev==1)? yellow(0.8f) : red(0.8f)));
	if (hasMessage2 && Engine::Button(Display::width * 0.9f - 16, 1, 16, 16, Icons::details, white(0.8f)) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::SYSMSG;
	}

	UI::font.Align(ALIGN_TOPRIGHT);
	UI::Label(Display::width - 5.f, 3, 10, VERSIONSTRING, white(0.7f));
	UI::font.Align(ALIGN_TOPLEFT);
}

void VisSystem::DrawBar() {
	UI2::BackQuad(0, Display::height - 18.f, (float)Display::width, 18, white(1, 0.5f));
	UI::Quad(0, Display::height - 18.f, (float)Display::width, 18, black(0.5f));
	if (Particles::anim.frameCount > 1) {
		if (AnWeb::executing) {
			float w = 172;
			if (AnWeb::execFrame > 0) {
				UI::Quad(w, Display::height - 15.f, 150, 12, white(1, 0.05f));
				UI::Quad(w + 1, Display::height - 14.f, AnWeb::execFrame * 148.f  / Particles::anim.frameCount, 10, white(1, 0.7f));
				w += 155;
			}
			UI::Label(w, Display::height - 16.f, 12, "Running analysis...", white(0.5f));
		}
		else {
			if (!LiveSyncer::activeRunner) {
				if (!UI::editingText) {
					if (Input::KeyDown(KEY::RightArrow)) {
						Particles::IncFrame(false);
					}
					if (Input::KeyDown(KEY::LeftArrow)) {
						if (!!Particles::anim.currentFrame) Particles::SetFrame(Particles::anim.currentFrame - 1);
					}
				}
				if ((!UI::editingText && Input::KeyDown(KEY::Space)) || Engine::Button(100, Display::height - 17.f, 16, 16, Icons::right, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
					ParGraphics::animate = !ParGraphics::animate;
					ParGraphics::animOff = 0;
				}
			}

			auto& fps = ParGraphics::animTarFps;
			fps = TryParse(UI::EditText(120, Display::height - 17.f, 50, 16, 12, white(1, 0.4f), std::to_string(fps), true, white(), nullptr, std::to_string(fps) + " fps"), 0);
			fps = Clamp(fps, 0, 1000);

			auto ssz = Particles::anim.frameCount;
			float al = float(Particles::anim.currentFrame) / (ssz - 1);
			al = Engine::DrawSliderFill(175, Display::height - 13.f, Display::width - 310.f, 9, 0, 1, al, white(1, 0.3f), white(0));

			using fs = Particles::animdata::FRAME_STATUS;
			fs sold = Particles::anim.status[0];
			auto pw = float(Display::width - 310.f) / ssz;
			size_t p0 = 0;
			for (size_t p = 0; p <= ssz; ++p) {
				auto st = (p == ssz)? fs::UNLOADED : Particles::anim.status[p];
				if (st == fs::READING) st = fs::UNLOADED;
				if (sold != st || (p == ssz)) {
					if (sold == fs::LOADED)
						UI::Quad(175 + pw * p0, Display::height - 13.f, pw * (p-p0), 9, white(1, 0.5f));
					else if (sold == fs::BAD)
						UI::Quad(175 + pw * p0, Display::height - 13.f, pw * (p - p0), 9, red(1, 0.5f));
					sold = st;
					p0 = p;
				}
			}
			
			UI::Quad(172 + (Display::width - 310.f) * al, Display::height - 17.f, 6, 16, white());
			if ((Engine::Button(175, Display::height - 13.f, Display::width - 310.f, 9) & 0x0f) == MOUSE_DOWN)
				ParGraphics::seek = true;
			else ParGraphics::seek = ParGraphics::seek && Input::mouse0;

			if (ParGraphics::seek) {
				Particles::SetFrame((uint)roundf(al * (Particles::anim.frameCount - 1)));
			}

			UI::Label(Display::width - 130.f, Display::height - 16.f, 12, std::to_string(Particles::anim.currentFrame + 1) + "/" + std::to_string(Particles::anim.frameCount), white());
		}
	}
	else if (Particles::anim.reading && ParLoader::loadFrames) {
		float w = 172;
		if (*ParLoader::loadProgress > 0) {
			UI::Quad(w, Display::height - 15.f, 150, 12, white(1, 0.05f));
			UI::Quad(w + 1, Display::height - 14.f, 148 * *ParLoader::loadProgress, 10, white(1, 0.7f));
			w += 155;
		}
		UI::Label(w, Display::height - 16.f, 12, "Loading frame " + std::to_string(*ParLoader::loadFrames), white(0.5f));
	}
	else if (!!Particles::particleSz) {
		//UI::Label(172, Display::height - 16.f, 12, _("No Animation Data"), white(0.5f));
		if (Engine::Button(172, Display::height - 16.f, 150, 16, white(0.2f), _("Import Trajectory"), 12, white(1)) == MOUSE_RELEASE) {
			LBD_IMPORT
		}
	}
	/*
	byte sel = (byte)mouseMode;
	for (byte b = 0; b < 3; ++b) {
		if (Engine::Button(Display::width - 60.f + 17 * b, Display::height - 17.f, 16, 16, (&Icons::toolRot)[b], (sel == b) ? Vec4(1, 0.7f, 0.4f, 1) : white(0.7f), white(), white(0.5f)) == MOUSE_RELEASE) {
			mouseMode = (VIS_MOUSE_MODE)b;
		}
	}
	*/
}

void VisSystem::DrawMsgPopup() {
	UI::IncLayer();
	UI::Quad(0, 0, static_cast<float>(Display::width), static_cast<float>(Display::height), black(0.7f));
	
	UI::Quad(Display::width * 0.5f - 200, Display::height * 0.5f - 50, 400, 100, white(0.95f, 0.15f));

 	if ((Engine::Button(Display::width * 0.5f + 120, Display::height * 0.5f + 33, 79, 16, white(1, 0.4f), "Close", 12, white(), true) == MOUSE_RELEASE) ||
		(Input::KeyDown(KEY::Escape) && UI::_layer == UI::_layerMax) || 
		(Input::mouse0State == 1 && !Rect(Display::width*0.5f - 200, Display::height*0.5f - 50, 400, 100).Inside(Input::mousePos))) {
		Popups::type = POPUP_TYPE::NONE;
	}

	UI2::LabelMul(Display::width * 0.5f - 195, Display::height * 0.5f - 45, 12, message2);
}

void VisSystem::BlurBack() {
	if (!blur) return;
	const auto& cam = ChokoLait::mainCamera;
	const auto w = Display::width * Display::dpiScl;
	const auto h = Display::height * Display::dpiScl;
	if (blurRad > 1) {
		Effects::Blur(cam->blitFbos[0], cam->blitFbos[2], cam->blitFbos[1], cam->blitTexs[0], cam->blitTexs[2],
		blurRad-1, w, h);
		Effects::Blur(cam->blitFbos[1], cam->blitFbos[2], cam->blitFbos[1], cam->blitTexs[1], cam->blitTexs[2],
		1, w, h);
	}
	else {
		Effects::Blur(cam->blitFbos[0], cam->blitFbos[2], cam->blitFbos[1], cam->blitTexs[0], cam->blitTexs[2],
		blurRad, w, h);
	}
}

void VisSystem::Save(const std::string& path) {
	if (Particles::empty) return;
	Debug::Message("System", "Saving...");
	currentSavePath = path;
	IO::MakeDirectory(path + "_data/");
	XmlNode head("AViS_State_File");
	head.params.emplace("hash", version_hash);
	head.children.reserve(10);
	Particles::Serialize(head.addchild());
	ParGraphics::Serialize(head.addchild());
	AnWeb::Serialize(head.addchild());
	Xml::Write(&head, path + ".xml");
	Debug::Message("System", "Compressing...");
	std::string quiet = (Debug::suppress > 0) ? "-qq " : "";
	RunCmd::Run(EXPPATH "zip -m -j -r " + quiet + " \"" + path + EXT_SVFL "\" \"" + path + ".xml\" \"" + path + "_data\"");
	IO::RmDirectory(path + "_data/");
	currentSavePath = "";
	Debug::Message("System", "Save complete");
}

bool VisSystem::Load(const std::string& path) {
	bool success = false;
	Debug::Message("System", "Loading: " + path);
	Particles::Clear();
	AnWeb::Clear();
	currentSavePath = path;
	auto l = path.find_last_of('/');
	auto s = path.substr(0, l);
	auto nm = path.substr(l + 1);
	currentSavePath2 = path + "_temp__/";
	std::string quiet = (Debug::suppress > 0) ? "-qq " : "";
	auto res = RunCmd::Run(EXPPATH "unzip -o -j " + quiet + "-d \"" + path + "_temp__/\" \"" + path + EXT_SVFL "\"");
	if (!!res) {
		Debug::Warning("System", "extracting save file failed!");
		goto cleanup;
	}
	else {
		auto xml = Xml::Parse(currentSavePath2 + nm + ".xml");
		if (!xml) {
			Debug::Warning("System", "save file xml is corrupt!");
			goto cleanup;
		}
		else {
			auto n = &xml->children[0];
			Particles::Deserialize(n);
			ParGraphics::Deserialize(n);
			AnWeb::Deserialize(n);
			Debug::Message("System", "Load complete");
			success = true;
		}
	}
cleanup:
	IO::RmDirectory(currentSavePath2);
	currentSavePath = "";
	return success;
}

void VisSystem::RegisterPath() {
#ifdef PLATFORM_WIN

#else
	if (IO::HasFile("/usr/local/bin/avis")) {
		if (!tinyfd_messageBox("Register PATH", "/usr/local/bin/avis already exists.\nDo you want to override it?",
			"yesno", "warning", 0)) return;
		remove("/usr/local/bin/avis");
	}
	if (!!symlink((IO::path + "avis").c_str(), "/usr/local/bin/avis")) {
		tinyfd_messageBox("Register PATH", "Failed to create link to /usr/local/bin/avis", "ok", "error", 1);
	}
#endif
}
