#pragma once
#include "Engine.h"

typedef void(*stdioCallback)(std::string, bool); //bool iserror

/*! File/folder reading/writing functions.
[av] */
class IO {
public:
	static std::vector<std::string> GetFiles(const std::string& path, std::string ext = "");
	static void GetFolders(const std::string& path, std::vector<std::string>* names, bool hidden = false, bool all = false);
	static bool HasDirectory(std::string path);
	static void MakeDirectory(std::string path);
	static void RmDirectory(std::string path);
	static bool HasFile(std::string path);
	static std::string ReadFile(const std::string& path);
	static void WriteFile(const std::string& path, const std::string& data);
	static void HideInput(bool hide);
	static std::string GetText(const std::string& path);
	static std::vector<byte> GetBytes(const std::string& path);

	static bool IsRelPath(const std::string& path);
	static std::string ResolveUserPath(const std::string& path);
	static std::string FullPath(const std::string& path);

	static time_t ModTime(const std::string& path);

	static void StartReadStdio(std::string path, stdioCallback callback);
	static void FlushStdio();
	static void StopReadStdio();

	static void RedirectStdio2(std::string path);
	static void RestoreStdio2();

	static void OpenFd(std::string path);
	static void OpenEx(std::string path);

	static std::string path, userPath, currPath, tmpPath;

	static void InitPath();
	static std::wstring _tow(const std::string& s);
	static std::string _frw(const std::wstring& s);
	
private:
	static std::thread readstdiothread;
	static bool readingStdio;
	static int stdout_o, stderr_o;
	static FILE* stdout_n, *stderr_n;
	static std::string stdiop;
	static int waitstdio;
	static void DoReadStdio(stdioCallback cb);
};