#include "system.h"
#include "md/ParMenu.h"
#include "md/parloader.h"
#include "vis/pargraphics.h"
#include "vis/renderer.h"
#include "web/anweb.h"
#include "ui/icons.h"
#include "ui/help.h"
#include "ui/ui_ext.h"
#include "ui/localizer.h"
#include "res/resdata.h"
#include "live/livesyncer.h"
#include "utils/dialog.h"
#include "utils/xml.h"
#include "utils/runcmd.h"

#ifdef PLATFORM_WIN
#define EXPPATH "path=%path%;\"" + IO::path + "\"&&"
#else
#define EXPPATH
#endif

std::string VisSystem::version_hash =
#include "../../githash.h"
"";

Vec4 VisSystem::accentColor = Vec4(1, 0.75f, 0, 1);
float VisSystem::glass = 0.9f;
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
}

VIS_MOUSE_MODE VisSystem::mouseMode = VIS_MOUSE_MODE::ROTATE;

float VisSystem::_defBondLength = 0.0225f; // 0.15
std::unordered_map<uint, float> VisSystem::_bondLengths;
std::unordered_map<ushort, Vec3> VisSystem::_type2Col;
std::unordered_map<ushort, std::array<float, 2>> VisSystem::radii;

std::unordered_map<std::string, std::string> VisSystem::envs, VisSystem::prefs;

void VisSystem::Init() {
	message = _("Hello!");
	radii.clear();
	std::ifstream strm(IO::path + "config/radii.txt");
	if (strm.is_open()) {
		std::string s;
		while (!strm.eof()) {
			std::getline(strm, s);
			auto p = string_split(s, ' ', true);
			if (p.size() != 5) continue;
			auto i = *(ushort*)&(p[0])[0];
			auto ar = std::stof(p[1]);
			auto vr = std::stof(p[4]);
			radii.emplace(i, std::array<float,2>({{ar, vr}}));
		}
		strm.close();
	}
	strm.open(IO::path + "config/bondlengths.txt");
	_bondLengths.clear();
	if (strm.is_open()) {
		std::string s;
		while (!strm.eof()) {
			std::getline(strm, s);
			auto p = string_split(s, ' ', true);
			if (p.size() != 2) continue;
			auto p2 = string_split(p[0], '-');
			if (p2.size() != 2) continue;
			auto i1 = *(ushort*)&(p2[0])[0];
			auto i2 = *(ushort*)&(p2[1])[0];
			auto ln = std::pow(std::stof(p[1]) * 0.001f, 2);
			_bondLengths.emplace(i1 + (i2 << 16), ln);
			_bondLengths.emplace(i2 + (i1 << 16), ln);
		}
		strm.close();
	}
	strm.open(IO::path + "config/colors.txt");
	_type2Col.clear();
	if (strm.is_open()) {
		std::string s;
		Vec4 col;
		while (!strm.eof()) {
			std::getline(strm, s);
			auto p = string_split(s, ' ', true);
			if (p.size() != 4) continue;
			auto i = *(ushort*)&(p[0][0]);
			col.x = std::stof(p[1]);
			col.y = std::stof(p[2]);
			col.z = std::stof(p[3]);
			col.a = 1;
			_type2Col.emplace(i, col);
			Particles::colorPallete[Particles::defColPalleteSz] = 
				Particles::_colorPallete[Particles::defColPalleteSz] = col;
			Particles::defColPallete[Particles::defColPalleteSz++] = i;
		}
		strm.close();
	}
	for (int a = Particles::defColPalleteSz; a < 256; a++) {
		byte colb[3];
		Color::Hsv2Rgb(a * 0.66f / 255, 1, 1, colb[0], colb[1], colb[2]);
		Particles::colorPallete[a] =
			Particles::_colorPallete[a] = 
				Vec4(colb[0], colb[1], colb[2], 255) / 255.f;
	}

	auto& mi = menuItems[0];
	mi.resize(6);
	mi[0].Set(Icons::newfile, _("New"), []() {
		Particles::Clear();
		AnWeb::Clear();
	});
	mi[1].Set(Icons::openfile, _("Open"), []() {
		auto res = Dialog::OpenFile({ "*" EXT_SVFL });
		if (!!res.size()) {
			VisSystem::Load(res[0].substr(0, res[0].find_last_of('.')));
		}
	});
	mi[2].Set(0, _("Open Recent"), 0);
	auto& mic = mi[2].child;
	mic.resize(2);
	mic[0].Set(0, "boo", 0);
	mic[1].Set(0, "foo", 0);
	mi[3].Set(Icons::openfile, _("Import"), []() {
		ParLoader::OnOpenFile(Dialog::OpenFile(ParLoader::exts));
	});
	mi[4].Set(Icons::openfile, _("Import Recent"), 0);
	mi[5].Set(Icons::openfile, _("Import Remote"), []() {
		ParLoader::isSrv = true;
		const char* e = "";
		ParLoader::OnDropFile(1, &e);
	});

	auto& mi0 = menuItems[1];
	mi0.resize(2);
	mi0[0].Set(0, _("Config folder"), []() {
		IO::OpenFd(IO::path + "config");
	});
	mi0[1].Set(0, _("Nodes folder"), []() {
		IO::OpenFd(IO::path + "nodes");
	});

	auto& mi1 = menuItems[2];
	mi1.resize(3);
	mi1[0].Set(0, "Help I'm", 0);
	mi1[1].Set(0, "Trapped in", 0);
	mi1[2].Set(0, "Line " + std::to_string(__LINE__), 0);

	auto& mi2 = menuItems[3];
	mi2.resize(5);
	mi2[0].Set(0, "Image (GLSL)", []() {
		VisRenderer::ToImage();
	});
	mi2[1].Set(0, "Image (Raytraced)", 0);
	mi2[2].Set(0, "Movie (GLSL)", []() {
		VisRenderer::ToGif();
	});
	mi2[3].Set(0, "Movie (Raytraced)", 0);
	mi2[4].Set(0, "Options", 0);

	auto& mi3 = menuItems[4];
	mi3.resize(2);
	mi3[0].Set(0, "User Manual", []() {
		IO::OpenEx(IO::path + "docs/index.html");
	});
	mi3[1].Set(Icons::vis_atom, "Splash Screen", []() {
		ParMenu::showSplash = true;
	});
}

void VisSystem::InitEnv() {
	envs.clear();
	std::ifstream strm(IO::path + "config/env.txt");
	if (strm.is_open()) {
		std::string s;
		while (std::getline(strm, s)) {
			if (!s.size() || s[0] == '!') continue;
			auto lc = s.find_first_of('=');
			if (lc != std::string::npos) {
				envs.emplace(s.substr(0, lc), s.substr(lc + 1));
			}
		}
	}
	strm.close();
	prefs.clear();
	strm.open(IO::path + "config/preferences.txt");
	if (strm.is_open()) {
		std::string s;
		while (std::getline(strm, s)) {
			if (!s.size() || s[0] == '#') continue;
			auto lc = s.find_first_of('=');
			if (lc != std::string::npos) {
				prefs.emplace(s.substr(0, lc), s.substr(lc + 1));
			}
		}
	}
}

bool VisSystem::InMainWin(const Vec2& pos) {
	if (AnWeb::drawFull) return false;
	else return (pos.x > ParMenu::expandPos + 16) && (pos.x < Display::width - ((!Particles::particleSz || LiveSyncer::activeRunner)? LiveSyncer::expandPos : AnWeb::expandPos)) && (pos.y < Display::height - 18);
}

void VisSystem::UpdateTitle() {
	auto& c = menuItems[0][4].child;
	auto s = ParMenu::recentFiles.size();
	c.resize(s);
	for (size_t i = 0; i < s; i++) {
		c[i].Set(0, ParMenu::recentFilesN[i], []() {
			const char* cc[1] = { ParMenu::recentFiles[Popups::selectedMenu].c_str() };
			ParLoader::OnDropFile(1, cc);
			Popups::type = POPUP_TYPE::NONE;
		});
		c[i].icon = GLuint(-1);
	}
}

void VisSystem::DrawTitle() {
	UI::Quad(0,0, static_cast<float>(Display::width), 18, white(0.95f, 0.05f));
	const std::string menu[] = {_("File"), _("Edit"), _("Options"), _("Render"), _("Help")};
	bool iso = Popups::type == POPUP_TYPE::MENU && Popups::data >= menuItems && Popups::data < (menuItems + 5) && UI::_layerMax == UI::_layer+1;
	UI::ignoreLayers = iso; 
	for (uint i = 0; i < 5; i++) {
		auto st = Engine::Button(2.f + 60 * i, 1, 59, 16, white(0), menu[i], 12, white(), true);
		if (st == MOUSE_RELEASE || (!!(st & MOUSE_HOVER_FLAG) && iso)) {
			Popups::type = POPUP_TYPE::MENU;
			Popups::pos = Vec2(2 + 60 * i, 17);
			Popups::data = menuItems + i;
		}
	}
	UI::ignoreLayers = false;
	UI::Quad(Display::width * 0.6f, 1, Display::width * 0.3f, 16, white(1, 0.2f));
	UI::Label(Display::width * 0.6f + 2, 1, 12, message, (!messageSev) ? white(0.5f) : ((messageSev==1)? yellow(0.8f) : red(0.8f)));
	if (hasMessage2 && Engine::Button(Display::width * 0.9f - 16, 1, 16, 16, Icons::details, white(0.8f)) == MOUSE_RELEASE) {
		Popups::type = POPUP_TYPE::SYSMSG;
	}

	UI::font->Align(ALIGN_TOPRIGHT);
	UI::Label(Display::width - 5.f, 3, 10, VERSIONSTRING, white(0.7f));
	UI::font->Align(ALIGN_TOPLEFT);
}

void VisSystem::DrawBar() {
	UI::Quad(0, Display::height - 18.f, static_cast<float>(Display::width), 18, white(0.9f, 0.1f));
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
					if (Input::KeyDown(Key_RightArrow)) {
						Particles::IncFrame(false);
					}
					if (Input::KeyDown(Key_LeftArrow)) {
						if (!!Particles::anim.currentFrame) Particles::SetFrame(Particles::anim.currentFrame - 1);
					}
				}
				if ((!UI::editingText && Input::KeyDown(Key_Space)) || Engine::Button(150, Display::height - 17.f, 16, 16, Icons::right, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
					ParGraphics::animate = !ParGraphics::animate;
					ParGraphics::animOff = 0;
				}
			}

			auto& fps = ParGraphics::animTarFps;
			fps = TryParse(UI::EditText(170, Display::height - 17.f, 50, 16, 12, white(1, 0.4f), std::to_string(fps), true, white(), nullptr, std::to_string(fps) + " fps"), 0);
			fps = Clamp(fps, 0, 1000);

			auto ssz = Particles::anim.frameCount;
			float al = float(Particles::anim.currentFrame) / (ssz - 1);
			al = Engine::DrawSliderFill(225, Display::height - 13.f, Display::width - 385.f, 9, 0, 1, al, white(1, 0.3f), white(0));

			using fs = Particles::AnimData::FRAME_STATUS;
			fs sold = Particles::anim.status[0];
			auto pw = float(Display::width - 385.f) / ssz;
			size_t p0 = 0;
			for (size_t p = 0; p <= ssz; p++) {
				auto st = (p == ssz)? fs::UNLOADED : Particles::anim.status[p];
				if (st == fs::READING) st = fs::UNLOADED;
				if (sold != st || (p == ssz)) {
					if (sold == fs::LOADED)
						UI::Quad(225 + pw * p0, Display::height - 13.f, pw * (p-p0), 9, white(1, 0.5f));
					else if (sold == fs::BAD)
						UI::Quad(225 + pw * p0, Display::height - 13.f, pw * (p - p0), 9, red(1, 0.5f));
					sold = st;
					p0 = p;
				}
			}
			
			UI::Quad(222 + (Display::width - 385.f) * al, Display::height - 17.f, 6, 16, white());
			if ((Engine::Button(225, Display::height - 13.f, Display::width - 385.f, 9) & 0x0f) == MOUSE_DOWN)
				ParGraphics::seek = true;
			else ParGraphics::seek = ParGraphics::seek && Input::mouse0;

			if (ParGraphics::seek) {
				Particles::SetFrame((uint)roundf(al * (Particles::anim.frameCount - 1)));
			}

			UI::Label(Display::width - 155.f, Display::height - 16.f, 12, std::to_string(Particles::anim.currentFrame + 1) + "/" + std::to_string(Particles::anim.frameCount), white());
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
	else
		UI::Label(172, Display::height - 16.f, 12, _("No Animation Data"), white(0.5f));

	byte sel = (byte)mouseMode;
	for (byte b = 0; b < 3; b++) {
		if (Engine::Button(Display::width - 60.f + 17 * b, Display::height - 17.f, 16, 16, (&Icons::toolRot)[b], (sel == b) ? Vec4(1, 0.7f, 0.4f, 1) : white(0.7f), white(), white(0.5f)) == MOUSE_RELEASE) {
			mouseMode = (VIS_MOUSE_MODE)b;
		}
	}
}

void VisSystem::DrawMsgPopup() {
	UI::IncLayer();
	UI::Quad(0, 0, static_cast<float>(Display::width), static_cast<float>(Display::height), black(0.7f));
	
	UI::Quad(Display::width * 0.5f - 200, Display::height * 0.5f - 50, 400, 100, white(0.95f, 0.15f));

 	if ((Engine::Button(Display::width * 0.5f + 120, Display::height * 0.5f + 33, 79, 16, white(1, 0.4f), "Close", 12, white(), true) == MOUSE_RELEASE) ||
		(Input::KeyDown(Key_Escape) && UI::_layer == UI::_layerMax) || 
		(Input::mouse0State == 1 && !Rect(Display::width*0.5f - 200, Display::height*0.5f - 50, 400, 100).Inside(Input::mousePos))) {
		Popups::type = POPUP_TYPE::NONE;
	}

	UI2::LabelMul(Display::width * 0.5f - 195, Display::height * 0.5f - 45, 12, message2);
}

void VisSystem::Save(const std::string& path) {
	if (!Particles::particleSz) return;
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
