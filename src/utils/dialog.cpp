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

#include "dialog.h"
#include "tinyfiledialogs.h"

std::vector<std::string> Dialog::OpenFile(std::vector<std::string> pattern, bool mul) {
	auto ps = pattern.size();
	std::vector<const char*> pts(ps);
	for (size_t a = 0; a < ps; ++a) {
		pts[a] = pattern[a].c_str();
	}
	auto cres = tinyfd_openFileDialog("Open", nullptr, (int)ps, &pts[0], nullptr, mul);
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