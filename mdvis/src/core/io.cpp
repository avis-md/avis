#include "Engine.h"
#include <locale>
#include <codecvt>

#ifdef PLATFORM_WIN
#include <io.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#else
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#ifdef PLATFORM_OSX
#include <mach-o/dyld.h>
#endif
#endif

string IO::path = IO::InitPath();

std::thread IO::readstdiothread;
bool IO::readingStdio;
int IO::stdout_o, IO::stderr_o;
FILE* IO::stdout_n, *IO::stderr_n;
string IO::stdiop;
int IO::waitstdio;

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
	WIN32_FIND_DATAW fd;
	HANDLE hFind = FindFirstFileW(_tow(search_path).c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (hidden || !(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) && (fd.cFileName[0] != '.')) {
				names->push_back(_frw(fd.cFileName));
			}
		} while (::FindNextFileW(hFind, &fd));
		::FindClose(hFind);
	}
#else
	DIR* dir = opendir(&folder[0]);
	if (!dir) return;
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
	DWORD dwAttrib = GetFileAttributesW(_tow(szPath).c_str());
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
	CreateDirectoryW(_tow(szPath).c_str(), &sa);
#else
	mkdir(&szPath[0], 0777);
#endif
}

bool IO::HasFile(string szPath) {
#ifdef PLATFORM_WIN
	DWORD dwAttrib = GetFileAttributesW(_tow(szPath).c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES);
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

string IO::GetText(const string& path) {
	std::ifstream strm(path);
	std::stringstream ss;
	ss << strm.rdbuf();
	return ss.str();
}

std::vector<byte> IO::GetBytes(const string& path) {
	std::ifstream ifs(path, std::ios::binary | std::ios::ate);
	auto pos = ifs.tellg();

	std::vector<byte> res(pos);

	ifs.seekg(0, std::ios::beg);
	ifs.read((char*)&res[0], pos);

	return res;
}

int IO::ModTime(const string& s) {
	struct stat stt;
	auto rt = stat(s.c_str(), &stt);
	if (!!rt) return -1;
	return stt.st_mtime;
}

#ifndef PLATFORM_WIN
#define _dup dup
#define _dup2 dup2
#define _close close
#define _fileno fileno
#endif

void IO::StartReadStdio(string path, stdioCallback callback) {
	stdiop = path;
	string p = stdiop + ".out";
	string p2 = stdiop + ".err";
	readingStdio = true;
	stdout_o = _dup(1);
	stderr_o = _dup(2);
	#ifdef PLATFORM_WIN
	stdout_n = _fsopen(p.c_str(), "w", _SH_DENYWR);
	stderr_n = _fsopen(p2.c_str(), "w", _SH_DENYWR);
	//fopen_s(&stdout_n, p.c_str(), "w");
	#else
	stdout_n = fopen(p.c_str(), "w");
	stderr_n = fopen(p2.c_str(), "w");
	#endif
	_dup2(_fileno(stdout_n), 1);
	_dup2(_fileno(stderr_n), 2);
	
	readstdiothread = std::thread(DoReadStdio, callback);
}

void IO::FlushStdio() {
	fflush(stdout);
	fflush(stderr);
	waitstdio = 1;
	while (!!waitstdio);
}

void IO::StopReadStdio() {
	string p = stdiop + ".out";
	string p2 = stdiop + ".err";

	fflush(stdout);
	fflush(stderr);
	fclose(stdout_n);
	fclose(stderr_n);
	_dup2(stdout_o, 1);
	_dup2(stderr_o, 2);
	_close(stdout_o);
	_close(stderr_o);
	readingStdio = false;
	if (readstdiothread.joinable()) readstdiothread.join();
	remove(p.c_str());
	remove(p2.c_str());
}

void IO::OpenEx(string path) {
	//path = "\"" + path + "\"";
#ifdef PLATFORM_WIN
	ShellExecuteW(0, 0, _tow(path).c_str(), 0, 0, SW_SHOW);
#else
	//auto res = fork();
	//if (!res) {
	//	char* fs[] = { &path[0], 0 };
		//execv(
#if defined(PLATFORM_LNX)
		system(("xdg-open " + path).c_str());
#else
		//execl("open", &path[0]);
		system(("open " + path).c_str());
#endif
		//, fs);
	//}
	//else if (res == -1) return;
#endif
}

string IO::InitPath() {
	char cpath[200];
#ifdef PLATFORM_WIN
	GetModuleFileName(NULL, cpath, 200);
	string path2 = cpath;
	path2 = path2.substr(0, path2.find_last_of('\\') + 1);
	std::replace(path2.begin(), path2.end(), '\\', '/');
#elif defined(PLATFORM_LNX)
	readlink("/proc/self/exe", cpath, 200);
	string path2 = cpath;
	path2 = path2.substr(0, path2.find_last_of('/') + 1);
#else
	uint32_t sz = sizeof(cpath);
	_NSGetExecutablePath(cpath, &sz);
	string path2 = realpath(cpath, 0);
	path2 = path2.substr(0, path2.find_last_of('/') + 1);
#endif
	path = path2;
	//Debug::Message("IO", "Path set to " + path);
	return path2;
}

std::wstring IO::_tow(const string& s) {
	return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(s);
}

string IO::_frw(const std::wstring& s) {
	return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(s);
}

void IO::DoReadStdio(stdioCallback cb) {
	string p = stdiop + ".out";
	string p2 = stdiop + ".err";

	std::ifstream strm(p);
	std::ifstream strm2(p2);
	std::string line;
	if (!strm.is_open() || !strm2.is_open())
		Debug::Warning("IO", "Cannot open file for stdout / stderr!");
	else {
		bool has = false;
		while (readingStdio || has) {
			if (!!waitstdio) waitstdio = 2;
			has = false;
			if (std::getline(strm, line)) {
				has = true;
				cb(line, false);
			}
			else {
				if (!strm.eof()) {
					Debug::Warning("IO", "Failed to read from redirected stdout!");
					break;
				}
				strm.clear();
			}
			if (std::getline(strm2, line)) {
				has = true;
				cb(line, true);
			}
			else {
				if (!strm2.eof()) {
					Debug::Warning("IO", "Failed to read from redirected stdout!");
					break;
				}
				strm2.clear();
			}
			if (!has) {
				if (waitstdio == 2)
					waitstdio = 0;
				Engine::Sleep(100);
			}
		}
	}
	strm.close();
	strm2.close();
}