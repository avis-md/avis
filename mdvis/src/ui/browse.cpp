#include "browse.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"
#include "md/parloader.h"

Browse::MODE Browse::mode = Browse::MODE::NONE;
BrowseTarget* Browse::system;
int Browse::selId = -1;

fileCallback Browse::callback;

void Browse::Init() {
	
}

void Browse::Draw() {
	if (mode == MODE::NONE) return;
	UI::IncLayer();

	UI::Quad(0, 0, Display::width, Display::height, white(0.9f, 0.1f));

	if (Engine::Button(10, 2, 100, 16, Vec4(0.4f, 0.4f, 0.2f, 1), "Cancel", 12, white(), true) == MOUSE_RELEASE) {
		mode = MODE::NONE;
		delete(system);
		system = nullptr;
	}

	UI2::sepw = 0.33f;
	static std::string opts[] = { "Local", "Remote", "" };
	static uint issv = ParLoader::isSrv;
	static Popups::DropdownItem di(&issv, &opts[0]);
	UI2::Dropdown(115, 2, 170, "Location", di);
	if (ParLoader::isSrv != !!issv) {
		ParLoader::isSrv = !!issv;
		delete(system);
		if (!ParLoader::isSrv) system = new LocalBrowseTarget();
		else if (ParLoader::srv.ok) system = new RemoteBrowseTarget();
		else system = nullptr;
	}

	if (ParLoader::isSrv) {
		if (ParLoader::srv.ok) {
			if (Engine::Button(290, 2, 80, 16, red(1, 0.5f), "Disconnect", 12, white(), true) == MOUSE_RELEASE) {
				ParLoader::SrvDisconnect();
				delete(system);
				system = nullptr;
			}
		}
		else {
			if (Engine::Button(544, 2, 80, 16, green(1, 0.5f), "Connect", 12, white(), true) == MOUSE_RELEASE) {
				ParLoader::SrvConnect();
				if (ParLoader::srv.ok) {
					system = new RemoteBrowseTarget();
				}
			}
			static std::string opts2[] = { "Public Key", "Password", "" };
			static uint ispw = ParLoader::srvusepass;
			static Popups::DropdownItem di2(&ispw, &opts2[0]);
			ParLoader::srvusepass = !!ispw;
			UI2::Dropdown(290, 2, 150, "Auth", di2);
			
			UI2::sepw = 0.3f;
			ParLoader::srvuser = UI2::EditText(115, 19, 130, "Host", ParLoader::srvuser);
			UI2::sepw = 0.2f;
			ParLoader::srvhost = UI2::EditText(250, 19, 250, "Server", ParLoader::srvhost);
			UI2::sepw = 0.3f;
			ParLoader::srvport = TryParse(UI2::EditText(505, 19, 120, "Port", std::to_string(ParLoader::srvport)), 22);
			UI2::sepw = 0.25f;
			if (ParLoader::srvusepass) {
				ParLoader::srvpass = UI2::EditPass(115, 36, 300, "Password", ParLoader::srvpass);
			}
			else {
				ParLoader::srvkey = UI2::EditText(115, 36, 300, "Public Key", ParLoader::srvkey);
				ParLoader::srvpass = "";
			}
		}
	}

	if (system) {
		if (Engine::Button(10, 20, 16, 16, Icons::up) == MOUSE_RELEASE) {
			system->Seek("..", false);
			selId = -1;
		}
		if (Engine::Button(27, 20, 16, 16, Icons::help) == MOUSE_RELEASE) {
			system->Home();
			selId = -1;
		}
		if (Engine::Button(44, 20, 16, 16, Icons::refresh) == MOUSE_RELEASE) {
			system->Seek(system->path, true);
			selId = -1;
		}
		Engine::Button(115, 20, Display::width - 226, 16, white(1, 0.2f), 
			(selId > -1)? system->path + system->files[selId] : system->path, 12, white());

		if (selId > -1) {
			if (Engine::Button(Display::width - 110, 20, 100, 16, Vec4(0.2f, 0.4f, 0.2f, 1), "Open", 12, white(), true) == MOUSE_RELEASE) {
				mode = MODE::NONE;
				if (callback) callback(system->path + system->files[selId]);
				selId = -1;
				delete(system);
				system = nullptr;
				return;
			}
		}

		float off = UI::BeginScroll(10, 60, 100, Display::height - 80);
		for (auto& fd : system->fds) {
			if (Engine::Button(10, off, 100, 16, white(1, 0.2f), fd, 12, white()) == MOUSE_RELEASE) {
				system->Seek(fd, false);
				selId = -1;
			}
			off += 17;
		}
		UI::EndScroll(off);

		off = UI::BeginScroll(115, 60, Display::width - 120, Display::height - 65);
		int wn = (int)std::floorf((Display::width - 120) / 160);
		float ws = (Display::width - 120) / wn;
		int wi = 0;
		float woff = 115;
		off -= 17;
		for (int i = 0, j = (int)system->files.size(); i < j; i++) {
			if (!(i%wn)) {
				off += 17;
				if (off < 0 || off > Display::height) {
					i += wn-1;
					continue;
				}
			}
			const auto& f = system->files[i];
			if (Engine::Button(woff, off, ws-1, 16, white((i == selId)? 0.3f : 0.05f + 0.05f * (wi%2)), f, 12, white()) == MOUSE_RELEASE) {
				if (Input::dbclick) {
					mode = MODE::NONE;
					if (callback) callback(system->path + f);
					selId = -1;
					delete(system);
					system = nullptr;
					goto end2;
				}
				else {
					selId = i;
				}
			}
			if (!((i+1)%wn)) {
				wi = 0;
				woff = 115;
			}
			else {
				++wi;
				woff += ws;
			}
		}
	end2:
		UI::EndScroll(off);
	}
}