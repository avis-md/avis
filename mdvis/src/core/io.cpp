#include "Engine.h"
#include <locale>
#include <codecvt>
#include "utils/runcmd.h"

#ifdef PLATFORM_WIN
#include <io.h>
#include <Shlobj.h>
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

std::string IO::path, IO::userPath, IO::currPath;

std::thread IO::readstdiothread;
bool IO::readingStdio;
int IO::stdout_o, IO::stderr_o;
FILE* IO::stdout_n, *IO::stderr_n;
std::string IO::stdiop;
int IO::waitstdio;

std::vector<std::string> IO::GetFiles(const std::string& folder, std::string ext) {
	if (folder == "") return std::vector<std::string>();
	std::vector<std::string> names;
	auto exts = ext.size();
#ifdef PLATFORM_WIN
	std::string search_path = folder + "/*" + ext;
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
#else
	DIR* dir = opendir(&folder[0]);
	struct dirent* ep;
	while ((ep = readdir(dir))) {
		std::string nm(ep->d_name);
		if (ep->d_type == DT_REG) {
			if (!exts || ((nm.size() > (exts + 1)) && (nm.substr(nm.size() - exts) == ext)))
				names.push_back(nm);
		}
	}
#endif
	return names;
}

void IO::GetFolders(const std::string& folder, std::vector<std::string>* names, bool hidden, bool all) {
	names->clear();
#ifdef PLATFORM_WIN
	std::string search_path = folder + "/*";
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
		std::string nm(ep->d_name);
		if (ep->d_type == DT_DIR && (all || (nm != "." && nm != ".."))) {
			names->push_back(std::string(ep->d_name));
		}
	}
#endif
}

bool IO::HasDirectory(std::string path) {
#ifdef PLATFORM_WIN
	DWORD dwAttrib = GetFileAttributesW(_tow(path).c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat st;
	return !stat(&path[0], &st);
#endif
}

void IO::MakeDirectory(std::string path) {
#ifdef PLATFORM_WIN
	SECURITY_ATTRIBUTES sa = {};
	sa.nLength = sizeof(sa);
	CreateDirectoryW(_tow(path).c_str(), &sa);
#else
	mkdir(&path[0], 0777);
#endif
}

void IO::RmDirectory(std::string path) {
#ifdef PLATFORM_WIN
	RunCmd::Run("rmdir /Q /S \"" + path + "\"");
#else
	RunCmd::Run("rm -rf \"" + path + "\"");
#endif
}

bool IO::HasFile(std::string path) {
#ifdef PLATFORM_WIN
	DWORD dwAttrib = GetFileAttributesW(_tow(path).c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES);
#else
	return (access(&path[0], F_OK ) != -1);
#endif
}

std::string IO::ReadFile(const std::string& path) {
	std::ifstream stream(path.c_str());
	if (!stream.good()) {
		Debug::Warning("IO", "Cannot read file: \"" + path + "\"!");
		return "";
	}
	std::stringstream buffer;
	buffer << stream.rdbuf();
	return buffer.str();
}

void IO::WriteFile(const std::string& path, const std::string& data) {
	std::ofstream strm(path);
	strm.write(data.data(), data.size());
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

std::string IO::GetText(const std::string& path) {
	std::ifstream strm(path);
	std::stringstream ss;
	ss << strm.rdbuf();
	return ss.str();
}

std::vector<byte> IO::GetBytes(const std::string& path) {
	std::ifstream ifs(path, std::ios::binary | std::ios::ate);
	auto pos = ifs.tellg();

	std::vector<byte> res((uint)pos);

	ifs.seekg(0, std::ios::beg);
	ifs.read((char*)&res[0], pos);

	return res;
}

bool IO::IsRelPath(const std::string& path) {
#ifdef PLATFORM_WIN
	return (path[1] != ':') || path[0] < 'A' || path[0] > 'Z';
#else
	return path[0] != '/';
#endif
}

std::string IO::ResolveUserPath(const std::string& path) {
	if (path[0] == '~') {
		if (path[1] == '/')
			return userPath + path.substr(1);
	}
	return path;
}

std::string IO::FullPath(const std::string& path) {
	auto p = ResolveUserPath(path);
	return IsRelPath(p) ? currPath + p : p;
}

time_t IO::ModTime(const std::string& s) {
	struct stat stt;
	auto rt = stat(s.c_str(), &stt);
	if (!!rt) return -1;
	return (int)stt.st_mtime;
}

#ifndef PLATFORM_WIN
#define _dup dup
#define _dup2 dup2
#define _close close
#define _fileno fileno
#endif

void IO::StartReadStdio(std::string path, stdioCallback callback) {
	stdiop = path;
	std::string p = stdiop + ".out";
	std::string p2 = stdiop + ".err";
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
	std::string p = stdiop + ".out";
	std::string p2 = stdiop + ".err";

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

void IO::RedirectStdio2(std::string path) {
	fflush(stdout);
	fflush(stderr);
	stdout_o = _dup(1);
	stderr_o = _dup(2);
#ifdef PLATFORM_WIN
	stdout_n = _fsopen(path.c_str(), "w", _SH_DENYWR);
	stdout_n = _fsopen(path.c_str(), "w", _SH_DENYWR);
#else
	stdout_n = fopen((path + "_o").c_str(), "w");
	stderr_n = fopen((path + "_e").c_str(), "w");
#endif
	_dup2(_fileno(stdout_n), 1);
	_dup2(_fileno(stderr_n), 2);
}

void IO::RestoreStdio2() {
	fflush(stdout);
	fflush(stderr);
	_dup2(stdout_o, 1);
	_dup2(stderr_o, 2);
	_close(stdout_o);
	_close(stderr_o);
	fclose(stdout_n);
	fclose(stderr_n);
}

void IO::OpenFd(std::string path) {
#if PLATFORM_WIN
	std::replace(path.begin(), path.end(), '/', '\\');
	RunCmd::Run("explorer \"" + path + "\"");
#elif PLATFORM_LNX
	RunCmd::Run("gnome-open \"" + path + "\"");
#else
	RunCmd::Run("open \"" + path + "\"");
#endif
}

void IO::OpenEx(std::string path) {
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

void IO::InitPath() {
	char cpath[200];
#ifdef PLATFORM_WIN
	GetModuleFileName(NULL, cpath, 200);
	path = cpath;
	path = path.substr(0, path.find_last_of('\\') + 1);
	auto p = path;
	std::replace(path.begin(), path.end(), '\\', '/');
#elif defined(PLATFORM_LNX)
	readlink("/proc/self/exe", cpath, 200);
	path = cpath;
	path = path.substr(0, path.find_last_of('/') + 1);
#else
	uint32_t sz = sizeof(cpath);
	_NSGetExecutablePath(cpath, &sz);
	path = realpath(cpath, 0);
	path = path.substr(0, path.find_last_of('/') + 1);
#endif
	if (path.back() != '/') path += "/";

#ifdef PLATFORM_WIN
	RunCmd::Run("cd>\"" + p + "\\currpath.txt\"");
	std::ifstream strm(path + "currpath.txt");
	std::getline(strm, currPath);
	std::replace(currPath.begin(), currPath.end(), '\\', '/');
	WCHAR path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) {
		userPath = IO::_frw(path);
		std::replace(userPath.begin(), userPath.end(), '\\', '/');
	}
#else
	RunCmd::Run("pwd>/tmp/avis_currpath.txt&&cd&&pwd>/tmp/avis_userpath.txt");
	std::ifstream strm("/tmp/avis_currpath.txt");
	std::getline(strm, currPath);
	strm.close();
	strm.open("/tmp/avis_userpath.txt");
	std::getline(strm, userPath);
#endif
	if (currPath.back() != '/') currPath += "/";
	if (userPath.back() != '/') userPath += "/";
	Debug::Message("IO", "Working path is \"" + currPath + "\"");
	Debug::Message("IO", "User path is \"" + userPath + "\"");
}

std::wstring IO::_tow(const std::string& s) {
	return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(s);
}

std::string IO::_frw(const std::wstring& s) {
	return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(s);
}

void IO::DoReadStdio(stdioCallback cb) {
	std::string p = stdiop + ".out";
	std::string p2 = stdiop + ".err";

	std::ifstream strm(p);
	std::ifstream strm2(p2);
	std::string line;
	std::streampos pos = 0, pos2 = 0;
	if (!strm.is_open() || !strm2.is_open()) {
		Debug::Warning("IO", "Cannot open file for stdout / stderr!");
		return;
	}
	while (readingStdio) {
		bool has = false;
		if (!!waitstdio) waitstdio = 2;
		if (std::getline(strm, line)) {
			has = true;
			pos = strm.tellg();
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
			pos2 = strm2.tellg();
			cb(line, true);
		}
		else {
			if (!strm2.eof()) {
				Debug::Warning("IO", "Failed to read from redirected stderr!");
				break;
			}
			strm2.clear();
		}
		if (!has) {
			if (waitstdio == 2)
				waitstdio = 0;
			Engine::Sleep(50);
			strm.seekg(pos);
			strm2.seekg(pos2);
		}
	}
}