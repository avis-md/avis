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

#include "browsetarget.h"
#include "imp/parloader.h"

RemoteBrowseTarget::RemoteBrowseTarget(std::string p) {
	if (IO::HasDirectory(p)) path = p;
	else path = ParLoader::srv.userPath;
	Seek(path, true);
}

void RemoteBrowseTarget::Seek(std::string fd, bool isfull) {
	auto _p = path;
	if (fd == ".") return;
	else if (fd == "..") {
		if (fd == "/") return;
		path.pop_back();
		path = path.substr(0, path.find_last_of('/') + 1);
	}
	else {
		path = isfull? fd : path + fd + "/";
	}
	if (ParLoader::srv.HasFile(path)) {
		ParLoader::srv.ListFD(path, files, fds);

		std::sort(fds.begin(), fds.end());
		std::sort(files.begin(), files.end());
		if (!fds.size()) {
			fds.push_back(".");
			fds.push_back("..");
		}
	}
	else path = _p;
}

void RemoteBrowseTarget::Home() {
	Seek(ParLoader::srv.userPath, true);
}