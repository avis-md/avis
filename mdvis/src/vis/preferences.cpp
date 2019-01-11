#include "preferences.h"
#include "res/prefdata.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"
#include "utils/xml.h"

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
			prf.dval_b = (**s++ == '1');
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
		case 'C':
			prf.type = Pref::TYPE::COLOR;
			prf.dval_c.r = (float)std::atof(*s++);
			prf.dval_c.g = (float)std::atof(*s++);
			prf.dval_c.b = (float)std::atof(*s++);
			prf.dval_c.a = (float)std::atof(*s++);
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
	const float x1 = x0 + 105;
	const float w2 = 280;
	UI2::BackQuad(x0, y0, 400, 300);
	UI::Quad(x0, y0, 400, 16, black(0.5f));
	UI::Label(x0 + 2, y0, 12, "Preferences", white());
	UI::Quad(x0, y0 + 16, 100, 284, black(0.3f));
	if (Engine::Button(x0 + 384, y0, 16, 16, Icons::cross) == MOUSE_RELEASE)
		show = false;
	for (int a = 0; a < 3; ++a) {
		UI::Label(x0 + 5, y0 + 20 + 22 * a, 14, prefs[a].first, (a == menu)? VisSystem::accentColor : white());
		if (Engine::Button(x0, y0 + 20 + 22 * a, 100, 20) == MOUSE_RELEASE)
			menu = a;
	}

	auto& prf = prefs[menu].second;

	float off = UI::BeginScroll(x1, y0 + 18, w2, 264);
	for (auto& p : prf) {
		bool chg = false;
		switch (p.type) {
		case Pref::TYPE::BOOL: {
			if (!p.val_b) break;
			bool v = *p.val_b;
			UI2::Toggle(x1, off, w2, p.name, *p.val_b);
			chg = (v != *p.val_b);
			break;
		}
		case Pref::TYPE::INT: {
			if (!p.val_i) break;
			int v = *p.val_i;
			if (p.slide) {
				*p.val_i = UI2::SliderI(x1, off, w2, p.name, p.min_i, p.max_i, *p.val_i);
			}
			else {
				*p.val_i = TryParse(UI2::EditText(x1, off, w2, p.name, std::to_string(*p.val_i)), 0);
				if (p.minmax)
					*p.val_i = Clamp(*p.val_i, p.min_i, p.max_i);
			}
			chg = (v != *p.val_i);
			break;
		}
		case Pref::TYPE::FLOAT: {
			if (!p.val_f) break;
			float v = *p.val_f;
			if (p.slide) {
				*p.val_f = UI2::Slider(x1, off, w2, p.name, p.min_f, p.max_f, *p.val_f);
			}
			else {
				*p.val_f = TryParse(UI2::EditText(x1, off, w2, p.name, std::to_string(*p.val_f)), 0.f);
				if (p.minmax)
					*p.val_f = Clamp(*p.val_f, p.min_f, p.max_f);
			}
			chg = (v != *p.val_f);
			break;
		}
		case Pref::TYPE::STRING:

			break;
		case Pref::TYPE::COLOR:
			if (!p.val_c) break;
			UI2::Color(x1, off, w2, p.name, *p.val_c);
			if (p.val_co != *p.val_c) {
				p.val_co = *p.val_c;
				chg = true;
			}
			break;
		}
		if (chg) {
			p.changed = true;
			if (p.name.back() != '*') p.name.push_back('*');
			if (p.callback) p.callback();
		}
		off += 17;
	}
	UI::EndScroll(off);

	if (Engine::Button(x0 + 400 - 243, y0 + 283, 80, 16, white(0.4f), "Reset", 12, white(), true) == MOUSE_RELEASE) {
		Reset();
	}
	if (Engine::Button(x0 + 400 - 162, y0 + 283, 80, 16, white(0.4f), "Save", 12, white(), true) == MOUSE_RELEASE) {
		Save();
	}
	if (Engine::Button(x0 + 400 - 81, y0 + 283, 80, 16, white(0.4f), "Load", 12, white(), true) == MOUSE_RELEASE) {
		Load();
	}
}

void Preferences::Reset() {
	for (auto& ps : prefs) {
		for (auto& p : ps.second) {
			switch (p.type) {
			case Pref::TYPE::BOOL: {
				if (!p.val_b) break;
				*p.val_b = p.dval_b;
				break;
			}
			case Pref::TYPE::INT: {
				if (!p.val_i) break;
				*p.val_i = p.dval_i;
				break;
			}
			case Pref::TYPE::FLOAT: {
				if (!p.val_f) break;
				*p.val_f = p.dval_f;
				break;
			}
			case Pref::TYPE::STRING:
				if (!p.val_s) break;
				*p.val_s = p.dval_s;
				break;
			case Pref::TYPE::COLOR:
				if (!p.val_c) break;
				*p.val_c = p.val_co = p.dval_c;
				break;
			}
			p.changed = false;
			if (p.name.back() == '*') p.name.pop_back();
			if (p.callback) p.callback();
		}
	}
}

void Preferences::Save() {
	XmlNode head("Preferences");
	for (auto& ps : prefs) {
		auto nd = head.addchild(ps.first);
		for (auto& p : ps.second) {
			if (!p.changed) continue;
			switch (p.type) {
			case Pref::TYPE::BOOL:
				nd->addchild(p.sig, (*p.val_b)? "1" : "0");
				break;
			case Pref::TYPE::INT:
				nd->addchild(p.sig, std::to_string(*p.val_i));
				break;
			case Pref::TYPE::FLOAT:
				nd->addchild(p.sig, std::to_string(*p.val_f));
				break;
			case Pref::TYPE::STRING:
				nd->addchild(p.sig, *p.val_s);
				break;
			case Pref::TYPE::COLOR:
				nd->children.push_back(Xml::FromVec(p.sig, *p.val_c));
				break;
			}
			p.changed = false;
			if (p.name.back() == '*') p.name.pop_back();
		}
	}
	Xml::Write(&head, VisSystem::localFd + "preferences.xml");
}

void Preferences::Load() {
	XmlNode* head = Xml::Parse(VisSystem::localFd + "preferences.xml");
	if (!head && !head->children.size()) return;

	for (auto& nd : head->children[0].children) {
		for (auto& ps : prefs) {
			if (nd.name == ps.first) {
				for (auto& n : nd.children) {
					for (auto& p : ps.second) {
						if (n.name == p.sig) {
							switch (p.type) {
							case Pref::TYPE::BOOL: {
								if (!p.val_b) break;
								*p.val_b = (n.value == "1");
								break;
							}
							case Pref::TYPE::INT: {
								if (!p.val_i) break;
								*p.val_i = std::stoi(n.value);
								break;
							}
							case Pref::TYPE::FLOAT: {
								if (!p.val_f) break;
								*p.val_f = (float)std::stof(n.value);
								break;
							}
							case Pref::TYPE::STRING:
								if (!p.val_s) break;
								*p.val_s = n.value;
								break;
							case Pref::TYPE::COLOR:
								if (!p.val_c) break;
								Xml::ToVec(&n, p.val_co);
								*p.val_c = p.val_co;
								break;
							}
						}
					}
				}
			}
		}
	}

	for (auto& ps : prefs) {
		for (auto& p : ps.second) {
			p.changed = false;
			if (p.name.back() == '*') p.name.pop_back();
		}
	}

	delete(head);
}

void Preferences::Link(const std::string sig, bool* b, void (*cb)()) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_b = b;
			*b = p.dval_b;
			p.callback = cb;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, int* i, void (*cb)()) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_i = i;
			*i = p.dval_i;
			p.callback = cb;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, float* f, void (*cb)()) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_f = f;
			*f = p.dval_f;
			p.callback = cb;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, std::string* s, void (*cb)()) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_s = s;
			*s = p.dval_s;
			p.callback = cb;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, Vec4* c, void (*cb)()) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_c = c;
			p.val_co = *c;
			*c = p.dval_c;
			p.callback = cb;
			break;
		}
	}
}
