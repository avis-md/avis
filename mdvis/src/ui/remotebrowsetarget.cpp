#include "browsetarget.h"
#include "md/parloader.h"

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