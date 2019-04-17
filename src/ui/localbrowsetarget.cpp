#include "browsetarget.h"

LocalBrowseTarget::LocalBrowseTarget(std::string p) {
	if (IO::HasDirectory(p)) path = p;
	else path = IO::userPath;
	Seek(path, true);
}

void LocalBrowseTarget::Seek(std::string fd, bool isfull) {
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
	if (IO::HasDirectory(path)) {
		files = IO::GetFiles(path);
		IO::GetFolders(path, &fds, true, true);
		std::sort(fds.begin(), fds.end());
		std::sort(files.begin(), files.end());
		if (!fds.size()) {
			fds.push_back(".");
			fds.push_back("..");
		}
	}
	else path = _p;
}

void LocalBrowseTarget::Home() {
	Seek(IO::userPath, true);
}