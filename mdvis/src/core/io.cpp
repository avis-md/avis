#include "Engine.h"

#ifdef PLATFORM_WIN
#include "Commdlg.h"
#pragma comment(lib, "Comdlg32.lib")
#else
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#endif

string IO::path = IO::InitPath();

string IO::OpenFile(string ext) {
#ifdef PLATFORM_WIN
	char buf[1024]{};
	OPENFILENAME fn = {};
	fn.lStructSize = sizeof(fn);
	fn.hwndOwner = glfwGetWin32Window(Display::window);
	fn.lpstrFilter = ("\0*" + ext + "\0\0").c_str();
	fn.lpstrFile = buf;
	fn.nMaxFile = 1024;
	GetOpenFileName(&fn);
	return string(buf);
#else
	
#endif
}

std::vector<string> IO::GetFiles(const string& folder, string ext) {
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

void IO::GetFolders(const string& folder, std::vector<string>* names, bool hidden) {
	names->clear();
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

bool IO::HasDirectory(string szPath) {
#ifdef PLATFORM_WIN
	DWORD dwAttrib = GetFileAttributes(&szPath[0]);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat st;
	return !stat(&szPath[0], &st);
#endif
}

void IO::MakeDirectory(string szPath) {
#ifdef PLATFORM_WIN
	SECURITY_ATTRIBUTES sa = {};
	sa.nLength = sizeof(sa);
	CreateDirectory(&szPath[0], &sa);
#else
	mkdir(&szPath[0], 0777);
#endif
}

bool IO::HasFile(string szPath) {
#ifdef PLATFORM_WIN
	DWORD dwAttrib = GetFileAttributes(&szPath[0]);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES);// && (dwAttrib & FILE_ATTRIBUTE_NORMAL));
#else
	return (access(&szPath[0], F_OK ) != -1);
#endif
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

void IO::HideInput(bool hide) {
#ifdef PLATFORM_WIN
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    if(hide)
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;
    SetConsoleMode(hStdin, mode );
#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if(hide)
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

#ifdef PLATFORM_WIN
std::vector<string> IO::GetRegistryKeys(HKEY key) {
	TCHAR    achKey[255];
	TCHAR    achClass[MAX_PATH] = TEXT("");
	DWORD    cchClassName = MAX_PATH;
	DWORD	 size;
	std::vector<string> res;

	if (RegQueryInfoKey(key, achClass, &cchClassName, NULL, &size, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
		DWORD cbName = 255;
		for (uint i = 0; i < size; i++) {
			if (RegEnumKeyEx(key, i, achKey, &cbName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
				res.push_back(achKey);
			}
		}
	}
	return res;
}

std::vector<std::pair<string, string>> IO::GetRegistryKeyValues(HKEY hKey, DWORD numValues) {
	std::vector<std::pair<string, string>> vals;
	for (DWORD i = 0; i < numValues; i++)
	{
		char valueName[201];
		DWORD valNameLen = 200;
		DWORD dataType;
		byte data[501];
		DWORD dataSize = 500;

		auto val = RegEnumValue(hKey,
			i,
			valueName,
			&valNameLen,
			NULL,
			&dataType,
			data, &dataSize);

		if (!!val) break;
		vals.push_back(std::pair<string, string>(string(valueName), string((char*)data)));
	}

	return vals;
}
#endif

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
	std::replace(path2.begin(), path2.end(), '\\', '/');
#else
	getcwd(cpath, 199);
	string path2 = cpath;
#endif
	path = path2;
	Debug::Message("IO", "Path set to " + path);
	return path2;
}