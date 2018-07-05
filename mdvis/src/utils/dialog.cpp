#include "dialog.h"
#include "tinyfiledialogs.h"

std::vector<string> Dialog::OpenFile(std::vector<string> pattern, bool mul) {
	auto ps = pattern.size();
	std::vector<const char*> pts(ps);
	for (auto a = 0; a < ps; a++) {
		pts[a] = pattern[a].c_str();
	}
	auto cres = tinyfd_openFileDialog("Open File", NULL, ps, &pts[0], NULL, mul);
	if (cres) {
		auto s = string(cres);
		return string_split(s, '|', true);
	}
	return std::vector<string>();
}