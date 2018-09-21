#pragma once
#include "Engine.h"

/*! Debugging functions. Output is saved in Log.txt beside the executable.
[av] */
class Debug {
public:
	static void Message(std::string c, std::string s);
	static void Warning(std::string c, std::string s);
	static void Error(std::string c, std::string s);

	static uint StackTrace(uint count, void** buffer);

	static byte suppress;

	friend class ChokoLait;
protected:
	static std::ofstream* stream;
	static void Init();

private:
#ifdef PLATFORM_WIN
	static HANDLE winHandle;
	static WORD winTextAttr;
#endif
};