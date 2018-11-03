#include "changelog.h"
#include "ui/ui_ext.h"

bool ChangeLog::show;
std::vector<std::string> ChangeLog::logs;

void ChangeLog::Init() {
	auto txt = IO::GetText(IO::path + "lastversion.txt");
	if (txt != APPVERSION) {
		IO::WriteFile(IO::path + "lastversion.txt", APPVERSION);
		show = true;
	}
	else show = false;
	auto s = IO::GetText(IO::path + "CHANGELOG");
	logs = string_split(s, '\n', true);
}

void ChangeLog::Draw() {
	if (!show) return;
	UI::IncLayer();
	UI::Quad(Display::width/2 - 200.f, Display::height/2 - 250.f, 400, 500, white(0.9f, 0.1f));
	UI::Label(Display::width/2 - 195.f, Display::height/2 - 247.f, 20, "Change Log " APPVERSION, white());
	UI::Quad(Display::width/2 - 195.f, Display::height/2 - 220.f, 390, 465, black());
	float off = Display::height/2 - 215.f;
	for (auto& l : logs) {
		if (l[0] == '#') {
			UI::Label(Display::width/2 - 193.f, off, 14, l.substr(1), white());
			off += 18;
		}
		else {
			UI::Label(Display::width/2 - 193.f, off, 12, l, white());
			off += 16;
		}
	}
	if ((UI::_layer == UI::_layerMax) && (Input::KeyDown(Key_Escape) || (Input::mouse0State == 1 && !Rect(Display::width/2 - 200.f, Display::height/2 - 250.f, 400, 500).Inside(Input::mousePos)))) {
		show = false;
	}
}