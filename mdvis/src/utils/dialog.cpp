#include "dialog.h"
#include "tinyfiledialogs.h"

std::vector<std::string> Dialog::OpenFile(std::vector<std::string> pattern, bool mul) {
	auto ps = pattern.size();
	std::vector<const char*> pts(ps);
	for (size_t a = 0; a < ps; ++a)  {
		pts[a] = pattern[a].c_str();
	}
	auto cres = tinyfd_openFileDialog("Open", nullptr, ps, &pts[0], nullptr, mul);
	if (cres) {
		auto s = std::string(cres);
#ifdef PLATFORM_WIN
		std::replace(s.begin(), s.end(), '\\', '/');
#endif
		return string_split(s, '|', true);
	}
	return std::vector<std::string>();
}

std::string Dialog::SaveFile(std::string ext) {
	const char* exts[] = { ext.c_str() };
	auto res = tinyfd_saveFileDialog("Save", nullptr, 1, exts, nullptr);
	return res? std::string(res) + ext : "";
}