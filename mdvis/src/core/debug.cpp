#include "Engine.h"

byte Debug::suppress = 0;

void Debug::Message(string c, string s) {
	if (stream) *stream << "[i]" << c << ": " << s << std::endl;
	if (suppress == 0)
		std::cout << "[i]" << c << ": " << s << std::endl;
}
void Debug::Warning(string c, string s) {
	if (stream) *stream << "[w]" << c << ": " << s << std::endl;
	if (suppress <= 1) {
#ifdef PLATFORM_WIN
		SetConsoleTextAttribute(winHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
		std::cout << "\033[33m";
#endif
		std::cout << "[w]" << c << ": " << s << std::endl;
#ifdef PLATFORM_WIN
		SetConsoleTextAttribute(winHandle, winTextAttr);
#else
		std::cout << "\033[0m";
#endif
	}
}
void Debug::Error(string c, string s) {
	if (stream) *stream << "[e]" << c << ": " << s << std::endl;
#ifdef PLATFORM_WIN
	SetConsoleTextAttribute(winHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
	std::cout << "\033[31m";
#endif
	std::cout << "[e]" << c << ": " << s << std::endl;
#ifdef PLATFORM_WIN
	SetConsoleTextAttribute(winHandle, winTextAttr);
#else
	std::cout << "\033[0m";
#endif
#ifdef PLATFORM_WIN
	__debugbreak();
#endif
}

uint Debug::StackTrace(uint count, void** frames) {
	return
#ifdef PLATFORM_WIN
		CaptureStackBackTrace(0, count, frames, NULL);
#else
		backtrace(frames, count);
#endif
}

std::ofstream* Debug::stream = nullptr;
#ifdef PLATFORM_WIN
	HANDLE Debug::winHandle = 0;
	WORD Debug::winTextAttr = 0;
#endif
void Debug::Init(string s) {
	string ss = s + "/Log.txt";
	stream = new std::ofstream(ss.c_str());
	Message("Debug", "Log Initialized");

#ifdef PLATFORM_WIN
	winHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(winHandle, &info);
	winTextAttr = info.wAttributes;
#endif
}