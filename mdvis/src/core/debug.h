#pragma once
#include "Engine.h"

/*! Debugging functions. Output is saved in Log.txt beside the executable.
[av] */
class Debug {
public:
	static void Message(string c, string s);
	static void Warning(string c, string s);
	static void Error(string c, string s);

	static uint StackTrace(uint count, void** buffer);

	static byte suppress;

	friend class ChokoLait;
protected:
	static std::ofstream* stream;
	static void Init(string path);

private:
#ifdef PLATFORM_WIN
	static HANDLE winHandle;
	static WORD winTextAttr;
#endif
};