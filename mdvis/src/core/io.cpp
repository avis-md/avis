#include "Engine.h"
#include "Editor.h"

#ifndef PLATFORM_WIN
#include <sys/types.h>
#include <dirent.h>
#endif

string IO::path = IO::InitPath();

std::vector<string> IO::GetFiles(const string& folder, string ext)
{
	if (folder == "") return std::vector<string>();
	std::vector<string> names;
	auto exts = ext.size();
#ifdef PLATFORM_WIN
	string search_path = folder + "/*" + ext;
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
#else //if defined(PLATFORM_ADR)
	DIR* dir = opendir(&folder[0]);
	struct dirent* ep;
	while ((ep = readdir(dir))) {
		string nm(ep->d_name);
		if (ep->d_type == DT_REG) {
			if (!exts || ((nm.size() > (exts + 1)) && (nm.substr(nm.size() - exts) == ext)))
				names.push_back(nm);
		}
	}
#endif
	return names;
}

void IO::GetFolders(const string& folder, std::vector<string>* names, bool hidden)
{
#ifdef PLATFORM_WIN
	string search_path = folder + "/*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (hidden || !(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) && (fd.cFileName[0] != '.')) {
				names->push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
#else
	DIR* dir = opendir(&folder[0]);
	struct dirent* ep;
	while ((ep = readdir(dir))) {
		string nm(ep->d_name);
		if (nm[0] != '.' && ep->d_type == DT_DIR) {
			names->push_back(string(ep->d_name));
		}
	}
#endif
}

bool IO::HasDirectory(string szPath)
{
#ifdef PLATFORM_WIN
	DWORD dwAttrib = GetFileAttributes(&szPath[0]);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
	return false;
}

bool IO::HasFile(string szPath)
{
#ifdef PLATFORM_WIN
	DWORD dwAttrib = GetFileAttributes(&szPath[0]);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES);// && (dwAttrib & FILE_ATTRIBUTE_NORMAL));
#endif
	return false;
}

string IO::ReadFile(const string& path) {
	std::ifstream stream(path.c_str());
	if (!stream.good()) {
		std::cout << "not found! " << path << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << stream.rdbuf();
	return buffer.str();
}

string IO::GetText(const string& path) {
	std::ifstream strm(path);
	std::stringstream ss;
	ss << strm.rdbuf();
	return ss.str();
}

string IO::InitPath() {
	char cpath[200];
#ifdef PLATFORM_WIN
	GetModuleFileName(NULL, cpath, 200);
	string path2 = cpath;
	path2 = path2.substr(0, path2.find_last_of('\\') + 1);
#else
	getcwd(cpath, 199);
	string path2 = cpath;
#endif
	path = path2;
	Debug::Message("IO", "Path set to " + path);
	return path2;
}