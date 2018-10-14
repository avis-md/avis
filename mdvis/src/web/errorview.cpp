#include "errorview.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"
#include "utils/runcmd.h"
#include "web/anweb.h"
#include "vis/system.h"

std::vector<ErrorView::Message> ErrorView::compileMsgs, ErrorView::execMsgs;

bool ErrorView::show = false, ErrorView::showExec = true;
int ErrorView::descId = -1;
int ErrorView::windowSize = 200, ErrorView::descSize = 68;

void _DoRefresh(AnBrowse::Folder& f) {
	for (auto& ff : f.subfolders)
		_DoRefresh(ff);
	for (auto& scr : f.scripts)
		ErrorView::compileMsgs.insert(ErrorView::compileMsgs.end(), scr->compileLog.begin(), scr->compileLog.end());
}

void ErrorView::Refresh() {
	compileMsgs.clear();
	_DoRefresh(AnBrowse::folder);
}

int ErrorView::Parse_GCC(const std::string& path, const std::string& sig, const std::string& name, std::vector<Message>& msgs) {
    msgs.clear();
    std::ifstream strm(path);
    std::string str;
    int n = 0;
    Message* msg = nullptr;
    const auto sz = sig.size();
    const std::string nm = name.substr(name.find_last_of('/') + 1);
    while (std::getline(strm, str)) {
#ifdef PLATFORM_WIN
		if (str[1] == ':' && str[0] >= 'A' && str[0] <= 'Z') {
#else
        if (str[0] == '/') {
#endif
			if (str.substr(0, sz) == sig) {
				if (string_find(str, "initialized and declared") > -1 || 
					string_find(str, "all code is position independent") > -1 ) continue;
				msgs.push_back(Message());
				msg = &msgs.back();
				msg->name = nm; msg->path = sig;
				int i = str.find_first_of(':', sz + 2);
				msg->linenum = TryParse(str.substr(sz + 1, i - sz - 1), -1);
				if (msg->linenum > -1) {
					int j = str.find_first_of(':', i + 2);
					msg->charnum = TryParse(str.substr(i + 1, j - i - 1), -1);
					str = str.substr(j + 2);
					if (str.substr(0, 5) == "error") {
						msg->severe = true;
						msg->msg.resize(1, str.substr(7));
						n++;
					}
					else {
						msg->msg.resize(1, str.substr(9));
						
					}
				}
				else {
					msg = nullptr;
					msgs.pop_back();
				}
			}
            else msg = nullptr;
        }
        else if (str.size() > 11 && str.substr(str.size() - 10) == "generated.") {
            return n;
        }
        else if (msg)
            msg->msg.push_back(str);
    }
    return n;
}

int ErrorView::Parse_MSVC(const std::string& path, const std::string& sig, const std::string& name, std::vector<Message>& msgs) {
	msgs.clear();
	std::ifstream strm(path);
	std::string str;
	int n = 0;
	Message* msg = nullptr;
	const auto sz = sig.size();
	const std::string nm = name.substr(name.find_last_of('/') + 1);
	while (std::getline(strm, str)) {
		if (str[0] == ' ' && msg)
			msg->msg.push_back(str);
		else if (str.substr(0, sz) == sig) {
			msgs.push_back(Message());
			msg = &msgs.back();
			msg->name = nm; msg->path = sig;
			int i = str.find_first_of(')', sz + 2);
			msg->linenum = TryParse(str.substr(sz + 1, i - sz - 1), -1);
			str = str.substr(i + 4);
			if (str.substr(0, 5) == "error") {
				msg->severe = true;
				msg->msg.resize(1, str.substr(6));
				n++;
			}
			msg->msg.resize(1, str.substr(8));
		}
		else msg = nullptr;
	}
	return n;
}

void ErrorView::Draw() {
	if (AnBrowse::busy) {
		Engine::RotateUI(Time::time * 180, Vec2(13, Display::height - 9));
		UI::Texture(5, Display::height - 17.f, 16, 16, Icons::refresh);
		Engine::ResetUIMatrix();
	}
	else {
		if (Engine::Button(3, Display::height - 17.f, 35, 16, white(0), white(0.2f), black(0.2f)) == MOUSE_RELEASE) {
			if (show && !showExec) show = false;
			else {
				show = true;
				showExec = false;
			}
			descId = -1;
		}
		UI::Texture(5, Display::height - 17.f, 16, 16, Icons::compile);
		auto csz = compileMsgs.size();
		UI::Label(22, Display::height - 17.f, 12, std::to_string(csz), (!csz) ? white(0.8f) : red());
		if (Engine::Button(40, Display::height - 17.f, 35, 16, white(0), white(0.2f), black(0.2f)) == MOUSE_RELEASE) {
			if (show && showExec) show = false;
			else {
				show = true;
				showExec = true;
			}
			descId = -1;
		}
		UI::Texture(42, Display::height - 17.f, 16, 16, Icons::exec);
		auto msz = execMsgs.size();
		UI::Label(58, Display::height - 17.f, 12, std::to_string(msz), (!msz) ? white(0.8f) : red());
	}

	if (show) {
		float y = Display::height - 18.f - windowSize;
		UI::Quad(0, y, static_cast<float>(Display::width), static_cast<float>(windowSize), white(0.95f, 0.1f));
		y++;
		if (Engine::Button(2, y, 80, 16, white(0), white(0.2f), black(0.2f), "Compile Log", 12, UI::font, showExec ? white(0.8f) : VisSystem::accentColor) == MOUSE_RELEASE) {
			showExec = false;
			descId = -1;
		}
		if (Engine::Button(85, y, 80, 16, white(0), white(0.2f), black(0.2f), "Exec Log", 12, UI::font, showExec ? VisSystem::accentColor : white(0.8f)) == MOUSE_RELEASE) {
			showExec = true;
			descId = -1;
		}
		if (Engine::Button(Display::width - 18.f, y, 16, 16, Icons::cross) == MOUSE_RELEASE) {
			show = false;
		}
		y += 19;
		std::vector<Message>& msgs = showExec ? execMsgs : compileMsgs;
		for (size_t a = 0; a < msgs.size(); a++) {
			auto& err = msgs[a];
			if (Engine::Button(0, y + a * 17, static_cast<float>(Display::width), 17, white((a == descId) ? 0.1f : 0), white(0.2f), black(0.1f)) == MOUSE_RELEASE) {
				if (Input::dbclick) {
#ifdef PLATFORM_WIN
					RunCmd::Run("\"" + err.path + "\"");
#else
					RunCmd::Run("open " + err.path);
#endif
				}
				else if (descId == a) descId = -1;
				else descId = a;
			}
			if (err.severe) {
				UI::Texture(5, y + a * 17 + 1, 14, 14, Icons::cross, red());
			}
			UI::Label(25, y + a * 17, 12, err.name, white());
			UI::Label(Display::width * 0.15f, y + a * 17, 12, "(" + std::to_string(err.linenum) + ":" + std::to_string(err.charnum) + ") " + err.msg[0], white());
		}

		if (descId > -1) {
			y = Display::height - 18.f - descSize;
			UI::Quad(0, y, static_cast<float>(Display::width), (float)descSize, white(0.95f, 0.15f));
			int a = 0;
			auto& msg = msgs[descId];
			for (auto& m : msg.msg) {
				UI::Label(5, y + 17 * a, 12, m, white());
				a++;
			}
			//UI::Label(100, Display::height - 16.f - windowSize, 12, msg.path, white(0.7f));
		}
		else {
			UI::Quad(0, Display::height - 35.f, static_cast<float>(Display::width), 17, white(0.95f, 0.15f));
			UI::Label(5, Display::height - 35.f, 12, "Select an error message for details", white(0.7f));
		}
	}
}

int ErrorView::Parse_GFortran(const std::string& path, const std::string& sig, const std::string& name, std::vector<Message>& msgs) {
    msgs.clear();
    std::ifstream strm(path);
    std::string str;
    int n = 0;
    Message* msg = nullptr;
    const auto sz = sig.size();
    const std::string nm = name.substr(name.find_last_of('/') + 1);
    while (std::getline(strm, str)) {
#ifdef PLATFORM_WIN
		if (str[1] == ':' && str[0] >= 'A' && str[0] <= 'Z') {
#else
        if (str[0] == '/') {
#endif
			if (str.substr(0, sz) == sig) {
				msgs.push_back(Message());
				msg = &msgs.back();
				msg->name = nm; msg->path = sig;
				int i = str.find_first_of(':', sz + 2);
				msg->linenum = TryParse(str.substr(sz + 1, i - sz - 1), -1);
				if (msg->linenum > -1) {
					int j = str.find_first_of(':', i + 2);
					msg->charnum = TryParse(str.substr(i + 1, j - i - 1), -1);
					msg->msg.resize(1);
					continue;
				}
				else {
					msg = nullptr;
					msgs.pop_back();
				}
			}
        }
        if (msg) {
			if (str.substr(0, 7) == "Error: ") {
				msg->msg[0] = str.substr(7);
				msg->severe = true;
				msg = nullptr;
			}
			else if (str.substr(0, 7) == "Warning: ") {
				msg->msg[0] = str.substr(7);
				msg = nullptr;
			}
			else
	            msg->msg.push_back(str);
		}
    }
    return n;
}