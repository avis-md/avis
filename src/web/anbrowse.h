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

#pragma once
#include "anweb.h"

class AnBrowse {
public:
	static bool busy, changed;
	static std::string busyMsg;

	static struct Folder {
		Folder(std::string nm) : name(nm) {}

		std::string name, fullName;
		bool expanded = true;
		std::vector<pAnScript> scripts;
		std::vector<Folder> subfolders;
		std::vector<std::string> saves;
	} folder;

	static bool expanded;
	static float expandPos;
	static bool mscFdExpanded[10];

	static Folder* doAddFd;
	static std::string tmplC, tmplP, tmplF;

	static void Init(), Update();

	static void Scan(), DoScan(Folder* f, const std::string& path, const std::string& incPath);
	static void Refresh(), DoRefresh(Folder* f);

	static void Draw(), DoDraw(Folder* f, float& off, uint layer);
};