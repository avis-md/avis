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

#include "preferences.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"
#include "utils/xml.h"
#include "res/prefdata.h"
#include "res/envdata.h"
#include "res/bnddata.h"
#include "res/coldata.h"
#include "res/raddata.h"

std::vector<std::pair<std::string, std::vector<Preferences::Pref>>> Preferences::prefs;

bool Preferences::showre = false;

std::vector<Preferences::Bondlen> Preferences::bondlengths;
std::vector<Preferences::Typecol> Preferences::typecolors;
std::vector<Preferences::Typerad> Preferences::typeradii;

bool Preferences::show = false;
int Preferences::menu = 0;

#define G2I(s) ((s == 'S')? 0 : ((s == 'A')? 1 : 2))

void Preferences::Init() {
	prefs.resize(5);
	prefs[0].first = "System";
	prefs[1].first = "Analysis";
	prefs[2].first = "Visualization";
	prefs[3].first = "Attributes";
	prefs[4].first = "Environment";

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

	s = (char**)config::env;
	while (*s) {
		prefs[4].second.push_back(Pref());
		auto& prf = prefs[4].second.back();
		prf.sig = *s++;
		prf.name = *s++;
		prf.type = Pref::TYPE::STRING;
		prf.dval_s = *s++;
		prf.desc = *s++;
	}

	s = (char**)config::bonds;
	while (*s) {
		bondlengths.push_back(Bondlen());
		auto& len = bondlengths.back();
		len.sig1 = *s++;
		len.sig2 = *s++;
		len.len = (float)std::atof(*s++) / 1000;
	}

	s = (char**)config::colors;
	while (*s) {
		typecolors.push_back(Typecol());
		auto& col = typecolors.back();
		col.sig = *s++;
		col.col.r = (float)std::atof(*s++);
		col.col.g = (float)std::atof(*s++);
		col.col.b = (float)std::atof(*s++);
		col.col.a = 1;
	}

	s = (char**)config::radii;
	while (*s) {
		typeradii.push_back(Typerad());
		auto& rad = typeradii.back();
		rad.sig = *s++;
		rad.rad = (float)std::atof(*s++);
	}
}

void Preferences::Draw() {
	if (!show) return;
	UI::IncLayer();
	UI::Quad(0, 0, Display::width, Display::height, black(0.4f));
	const float x0 = Display::width/2 - 200.f;
	const float y0 = Display::height/2 - 150.f;
	const float x1 = x0 + 105;
	const float w2 = 280;
	UI2::BackQuad(x0, y0, 400, 300);
	UI::Quad(x0, y0, 400, 16, black(0.5f));
	UI::Label(x0 + 2, y0, 12, "Preferences", white());
	UI::Quad(x0, y0 + 16, 100, 284, black(0.3f));
	if (Engine::Button(x0 + 384, y0, 16, 16, Icons::cross) == MOUSE_RELEASE)
		show = false;
	if ((UI::_layer == UI::_layerMax) && (Input::KeyDown(KEY::Escape) || (Input::mouse0State == 1 &&
			!Rect(x0, y0, 400, 300).Inside(Input::mousePos)))) {
		show = false;
	}
	
	for (int a = 0; a < 5; ++a) {
		UI::Label(x0 + 5, y0 + 20 + 22 * a, 14, prefs[a].first, (a == menu)? VisSystem::accentColor : white());
		if (Engine::Button(x0, y0 + 20 + 22 * a, 100, 20) == MOUSE_RELEASE)
			menu = a;
	}

	float off = 0;
	if (showre) {
		UI::Label(x1, y0 + 18, 12, "Environment variables are modified.", yellow());
		UI::Label(x1, y0 + 34, 12, "Consider restarting for changes to take effect.", yellow());
		off = UI::BeginScroll(x1, y0 + 18 + 33, 295, 264 - 33);
	}
	else off = UI::BeginScroll(x1, y0 + 18, 295, 264);

	if (menu == 3) {
		static bool sb = true;
		if (Engine::Button(x1, off, 16, 16, sb? Icons::collapse : Icons::expand) == MOUSE_RELEASE) sb = !sb;
		UI::Label(x1 + 17, off, 12, "Bond lengths", white()); off += 18;
		if (sb) {
			UI::Quad(x1, off - 2, w2, bondlengths.size() * 17 + 4, black(0.3f));
			for (auto& l : bondlengths) {
				l.sig1 = UI::EditText(x1 + 2, off, 50, 16, 12, white(1, 0.5f), l.sig1, true, white());
				l.sig2 = UI::EditText(x1 + 54, off, 50, 16, 12, white(1, 0.5f), l.sig2, true, white());
				l.len = TryParse(UI::EditText(x1 + 106, off, w2 - 108, 16, 12, white(1, 0.5f), std::to_string(l.len), true, white()), 0.0f);
				off += 17;
			}
			off += 4;
		}

		static bool sc = true;
		if (Engine::Button(x1, off, 16, 16, sc? Icons::collapse : Icons::expand) == MOUSE_RELEASE) sc = !sc;
		UI::Label(x1 + 17, off, 12, "Atom colors", white()); off += 18;
		if (sc) {
			UI::Quad(x1, off - 2, w2, typecolors.size() * 17 + 4, black(0.3f));
			for (auto& l : typecolors) {
				l.sig = UI::EditText(x1 + 2, off, 102, 16, 12, white(1, 0.5f), l.sig, true, white());
				UI2::Color(x1 + 106, off, w2 - 108, l.col);
				off += 17;
			}
			off += 4;
		}
		off++;

		static bool sr = true;
		if (Engine::Button(x1, off, 16, 16, sr? Icons::collapse : Icons::expand) == MOUSE_RELEASE) sr = !sr;
		UI::Label(x1 + 17, off, 12, "Atom radii", white()); off += 18;
		if (sr) {
			UI::Quad(x1, off - 2, w2, typeradii.size() * 17 + 4, black(0.3f));
			for (auto& l : typeradii) {
				l.sig = UI::EditText(x1 + 2, off, 102, 16, 12, white(1, 0.5f), l.sig, true, white());
				l.rad = TryParse(UI::EditText(x1 + 106, off, w2 - 108, 16, 12, white(1, 0.5f), std::to_string(l.rad), true, white()), 0.0f);
				off += 17;
			}
			off += 4;
		}
		off++;
	}
	else {
		auto& prf = prefs[menu].second;

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
			case Pref::TYPE::STRING: {
				if (!p.val_s) break;
				auto v = *p.val_s;
				UI2::sepw = 0.3f;
				*p.val_s = UI2::EditText(x1, off, w2, p.name, *p.val_s);
				UI2::sepw = 0.5f;
				chg = (v != *p.val_s);
				break;
			}
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
				if (menu == 4) showre = true;
			}
			off += 17;
		}

		if (menu == 0) {
			off += 2;
			if (Engine::Button(x1, off, w2, 16, white(1, 0.4f), "Install app in PATH", 12, white(), true) == MOUSE_RELEASE) {
				VisSystem::RegisterPath();
			}
		}
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
	for (int a = 0; a < 3; a++) {
		auto& ps = prefs[a];
		auto nd = head.addchild(ps.first);
		for (auto& p : ps.second) {
			switch (p.type) {
			case Pref::TYPE::BOOL:
				if (!p.val_b) break;
				nd->addchild(p.sig, (*p.val_b)? "1" : "0");
				break;
			case Pref::TYPE::INT:
				if (!p.val_i) break;
				nd->addchild(p.sig, std::to_string(*p.val_i));
				break;
			case Pref::TYPE::FLOAT:
				if (!p.val_f) break;
				nd->addchild(p.sig, std::to_string(*p.val_f));
				break;
			case Pref::TYPE::STRING:
				if (!p.val_s) break;
				nd->addchild(p.sig, *p.val_s);
				break;
			case Pref::TYPE::COLOR:
				if (!p.val_c) break;
				nd->children.push_back(Xml::FromVec(p.sig, *p.val_c));
				break;
			}
			p.changed = false;
			if (p.name.back() == '*') p.name.pop_back();
		}
	}
	Xml::Write(&head, VisSystem::localFd + "preferences.xml");

	SaveAttrs();
	SaveEnv();
}

void Preferences::Load() {
	XmlNode* head = Xml::Parse(VisSystem::localFd + "preferences.xml");
	if (!head || !head->children.size()) return;

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

	LoadAttrs();
	LoadEnv();
}

void Preferences::SaveAttrs() {
	XmlNode head("Attributes");

	auto len = head.addchild("bondlengths");
	for (auto& l : bondlengths) {
		auto i = len->addchild("item");
		i->addchild("sig1", l.sig1);
		i->addchild("sig2", l.sig2);
		i->addchild("value", std::to_string(l.len));
	}

	auto col = head.addchild("typecolors");
	for (auto& l : typecolors) {
		col->children.push_back(Xml::FromVec(l.sig, l.col));
	}

	auto rad = head.addchild("typeradii");
	for (auto& l : typeradii) {
		rad->addchild(l.sig, std::to_string(l.rad));
	}

	Xml::Write(&head, VisSystem::localFd + "attributes.xml");
}

void Preferences::LoadAttrs() {
	XmlNode* head = Xml::Parse(VisSystem::localFd + "attributes.xml");
	if (!head || !head->children.size()) return;

	auto& h = head->children[0].children;

	bondlengths.clear();
	for (auto& l : h[0].children) {
		if (l.name != "item") continue;
		bondlengths.push_back(Bondlen());
		auto& len = bondlengths.back();
		len.sig1 = l.children[0].value;
		len.sig2 = l.children[1].value;
		len.len = (float)std::stof(l.children[2].value);
	}

	typecolors.clear();
	for (auto& l : h[1].children) {
		typecolors.push_back(Typecol());
		auto& len = typecolors.back();
		len.sig = l.name;
		Xml::ToVec(&l, len.col);
	}

	typeradii.clear();
	for (auto& l : h[2].children) {
		typeradii.push_back(Typerad());
		auto& rad = typeradii.back();
		rad.sig = l.name;
		rad.rad = (float)std::stof(l.value);
	}

	delete(head);
}

void Preferences::SaveEnv() {
	XmlNode head("Environment");
	for (auto& p : prefs[4].second) {
		if (!p.val_s) continue;
		p.dval_s = *p.val_s;
		head.addchild(p.sig, p.dval_s);
		p.changed = false;
		if (p.name.back() == '*') p.name.pop_back();
	}
	Xml::Write(&head, VisSystem::localFd + "environment.xml");
}

void Preferences::LoadEnv() {
	XmlNode* head = Xml::Parse(VisSystem::localFd + "environment.xml");
	if (!head || !head->children.size()) return;

	for (auto& n : head->children[0].children) {
		for (auto& p : prefs[4].second) {
			if (n.name == p.sig) {
				p.dval_s = n.value;
				if (p.val_s) *p.val_s = p.dval_s;
				//std::replace(p.dval_s.begin(), p.dval_s.end(), '\\', '/');
			}
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

float Preferences::GetLen(ushort sig1, ushort sig2) {
	for (auto& l : bondlengths) {
		const ushort s1 = *(ushort*)l.sig1.data();
		const ushort s2 = *(ushort*)l.sig2.data();
		if ((s1 == sig1 && s2 == sig2) || (s1 == sig2 && s2 == sig1))
			return l.len;
	}
	return 0;
}

Vec3 Preferences::GetCol(ushort sig) {
	for (auto& c : typecolors) {
		const ushort s = *(ushort*)c.sig.data();
		if (s == sig) return (Vec3)c.col;
	}
	return white();
}

float Preferences::GetRad(ushort sig) {
	for (auto& r : typeradii) {
		const ushort s = *(ushort*)r.sig.data();
		if (s == sig) return std::max(r.rad, 0.0000001f);
	}
	return 1;
}

void Preferences::LinkEnv(const std::string sig, std::string* s) {
	auto& pp = prefs[4];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_s = s;
			*s = p.dval_s;
			break;
		}
	}
}
