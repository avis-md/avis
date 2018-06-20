#pragma once
#include "Engine.h"

typedef void(*stdioCallback)(string, bool); //bool iserror

/*! File/folder reading/writing functions.
[av] */
class IO {
public:
	static std::vector<string> GetFiles(const string& path, string ext = "");
	static void GetFolders(const string& path, std::vector<string>* names, bool hidden = false);
	static bool HasDirectory(string szPath);
	static void MakeDirectory(string szPath);
	static bool HasFile(string szPath);
	static string ReadFile(const string& path);
	static void HideInput(bool hide);
#if defined(PLATFORM_WIN)
	static std::vector<string> GetRegistryKeys(HKEY key);
	static std::vector<std::pair<string, string>> GetRegistryKeyValues(HKEY hKey, DWORD numValues = 5);
#endif
	static string GetText(const string& path);
	static std::vector<byte> GetBytes(const string& path);

	static void StartReadStdio(string path, stdioCallback callback);
	static void FlushStdio();
	static void StopReadStdio();

	static string path;

	static string InitPath();
	
private:
	static std::thread readstdiothread;
	static bool readingStdio;
	static int stdout_o, stderr_o;
	static FILE* stdout_n, *stderr_n;
	static string stdiop;
	static int waitstdio;
	static void DoReadStdio(stdioCallback cb);
};