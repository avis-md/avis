#pragma once
#include "Engine.h"

typedef void(*stdioCallback)(string, bool); //bool iserror

/*! File/folder reading/writing functions.
[av] */
class IO {
public:
	static std::vector<string> GetFiles(const string& path, string ext = "");
	static void GetFolders(const string& path, std::vector<string>* names, bool hidden = false);
	static bool HasDirectory(string path);
	static void MakeDirectory(string path);
	static bool HasFile(string path);
	static string ReadFile(const string& path);
	static void HideInput(bool hide);
	static string GetText(const string& path);
	static std::vector<byte> GetBytes(const string& path);

	static int ModTime(const string& path);

	static void StartReadStdio(string path, stdioCallback callback);
	static void FlushStdio();
	static void StopReadStdio();

	static void OpenEx(string path);

	static string path;

	static string InitPath();
	static std::wstring _tow(const string& s);
	static std::string _frw(const std::wstring& s);
	
private:
	static std::thread readstdiothread;
	static bool readingStdio;
	static int stdout_o, stderr_o;
	static FILE* stdout_n, *stderr_n;
	static string stdiop;
	static int waitstdio;
	static void DoReadStdio(stdioCallback cb);
};