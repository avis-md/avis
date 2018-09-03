#include "errorview.h"
#include "ui/icons.h"
#include "utils/runcmd.h"

std::vector<ErrorView::Message> ErrorView::compileMsgs, ErrorView::execMsgs;
int ErrorView::compileMsgSz = 0, ErrorView::execMsgSz = 0;

int ErrorView::descId = -1;
int ErrorView::windowSize = 200, ErrorView::descSize = 68;

int ErrorView::Parse_GCC(const string& path, const string& sig, const string& name, std::vector<Message>& msgs) {
    msgs.clear();
    std::ifstream strm(path);
    string str;
    int n = 0;
    Message* msg = nullptr;
    const auto sz = sig.size();
    const string nm = name.substr(name.find_last_of('/') + 1);
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

int ErrorView::Parse_MSVC(const string& path, const string& sig, const string& name, std::vector<Message>& msgs) {
	msgs.clear();
	std::ifstream strm(path);
	string str;
	int n = 0;
	Message* msg = nullptr;
	const auto sz = sig.size();
	const string nm = name.substr(name.find_last_of('/') + 1);
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
    float y = Display::height - 18.0f - windowSize;
    Engine::DrawQuad(0, y, (float)Display::width, (float)windowSize, white(0.95f, 0.1f));
    UI::Label(5, y + 2, 12, "Compile Log", white());
    y += 20;
    for (int a = 0; a < compileMsgSz; a++) {
        auto& err = compileMsgs[a];
        if (Engine::Button(0, y + a*17, (float)Display::width, 17, white((a == descId)? 0.1f : 0), white(0.2f), black(0.1f)) == MOUSE_RELEASE) {
            if (Input::dbclick) {
                RunCmd::Run("open " + err.path);
            }
            else if (descId == a) descId = -1;
            else descId = a;
        }
        if (err.severe) {
            UI::Texture(5, y + a*17 + 1, 14, 14, Icons::cross, red());
        }
        UI::Label(25, y + a*17, 12, err.name, white());
        UI::Label(Display::width * 0.15f, y + a*17, 12, "(" + std::to_string(err.linenum) + ":" + std::to_string(err.charnum) + ") " + err.msg[0], white());
    }

	if (descId > -1) {
		y = Display::height - 18.0f - descSize;
		Engine::DrawQuad(0, y, (float)Display::width, (float)descSize, white(0.95f, 0.15f));
		int a = 0;
		auto& msg = compileMsgs[descId];
        for (auto& m : msg.msg) {
            UI::Label(5, y + 17 * a, 12, m, white());
            a++;
        }
		UI::Label(100, Display::height - 16.0f - windowSize, 12, msg.path, white(0.7f));
	}
	else {
		Engine::DrawQuad(0, Display::height - 35.0f, (float)Display::width, 17, white(0.95f, 0.15f));
		UI::Label(5, Display::height - 35.0f, 12, "Select an error message", white(0.7f));
	}
}