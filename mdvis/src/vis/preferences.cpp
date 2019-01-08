#include "preferences.h"
#include "res/prefdata.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"

std::vector<std::pair<std::string, std::vector<Preferences::Pref>>> Preferences::prefs;
bool Preferences::show = false;
int Preferences::menu = 0;

#define G2I(s) ((s == 'S')? 0 : ((s == 'A')? 1 : 2))

void Preferences::Init() {
	prefs.resize(3);
	prefs[0].first = "System";
	prefs[1].first = "Analysis";
	prefs[2].first = "Visualization";

	char** s = (char**)config::prefs;
	int i = 0;

	while (*s) {
		i = G2I(**s);
		prefs[i].second.push_back(Pref());
		auto& prf = prefs[i].second.back();

		prf.sig = *s++;
		prf.name = *s++;

		const char typ = **s++;

		switch (typ) {
		case 'B':
			prf.type = Pref::TYPE::BOOL;
			prf.dval_i = (**s++ == '1');
			break;
		case 'I':
			prf.type = Pref::TYPE::INT;
			prf.dval_i = std::atoi(*s++);
			prf.min_i = std::atoi(*s++);
			prf.max_i = std::atoi(*s++);
			if (prf.min_i == prf.max_i) prf.minmax = false;
			else {
				prf.minmax = true;
				prf.slide = (**s++ == 'Y');
			}
			break;
		case 'F':
			prf.type = Pref::TYPE::FLOAT;
			prf.dval_f = (float)std::atof(*s++);
			prf.min_f = (float)std::atof(*s++);
			prf.max_f = (float)std::atof(*s++);
			if (prf.min_f == prf.max_f) prf.minmax = false;
			else {
				prf.minmax = true;
				prf.slide = (**s++ == 'Y');
			}
			break;
		case 'S':
			prf.type = Pref::TYPE::STRING;
			prf.dval_s = *s++;
			break;
		default:
			assert(0);
			break;
		}

		prf.desc = *s++;
	}
}

void Preferences::Draw() {
	if (!show) return;
	UI::IncLayer();
	const float x0 = Display::width*0.5f - 200.f;
	const float y0 = Display::height*0.5f - 150.f;
	UI2::BackQuad(x0, y0, 400, 300);
	UI::Quad(x0, y0, 100, 300, black(0.3f));
	if (Engine::Button(x0 + 384, y0, 16, 16, Icons::cross) == MOUSE_RELEASE)
		show = false;
	for (int a = 0; a < 3; ++a) {
		UI::Label(x0 + 5, y0 + 5 + 25 * a, 16, prefs[a].first, (a == menu)? VisSystem::accentColor : white());
		if (Engine::Button(x0, y0 + 5 + 25 * a, 100, 20) == MOUSE_RELEASE)
			menu = a;
	}

	auto& prf = prefs[menu].second;

	float off = UI::BeginScroll(x0 + 100, y0, 280, 300);
	for (auto& p : prf) {

	}
	UI::EndScroll(off);
}

void Preferences::Link(const std::string sig, bool* b) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_b = b;
			*b = p.dval_b;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, int* i) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_i = i;
			*i = p.dval_i;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, float* f) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_f = f;
			*f = p.dval_f;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, std::string* s) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_s = s;
			*s = p.dval_s;
			break;
		}
	}
}
