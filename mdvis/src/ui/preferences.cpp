#include "preferences.h"
#include "res/prefdata.h"

std::vector<std::pair<std::string, std::vector<Preferences::Pref>>> Preferences::prefs;
bool Preferences::show = false;

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

}

void Preferences::Link(const std::string sig, bool* b) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_b = b;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, int* i) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_i = i;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, float* f) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_f = f;
			break;
		}
	}
}

void Preferences::Link(const std::string sig, std::string* s) {
	auto& pp = prefs[G2I(sig[0])];
	for (auto& p : pp.second) {
		if (p.sig == sig) {
			p.val_s = s;
			break;
		}
	}
}
