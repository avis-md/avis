#include "pybrowse.h"

PyBrowse::Folder PyBrowse::folder = Folder("py");

void PyBrowse::DoScan(Folder* fo, const string& path, const string& incPath) {
	auto ff = IO::GetFiles(path, ".py");

	for (auto f : ff) {
		fo->scripts.push_back(nullptr);
		//PyReader::Read(IO::path + "/py/foo.py", &scr);
	}

	std::vector<string> fd;
	IO::GetFolders(path, &fd);

	for (auto f : fd) {
		if (f == "." || f == "..") continue;
		fo->subfolders.push_back(Folder(f));
		DoScan(&fo->subfolders.back(), path + "\\" + f, incPath + f + ".");
	}
}

void PyBrowse::Scan() {
	DoScan(&folder, IO::path + "\\py", "");
}