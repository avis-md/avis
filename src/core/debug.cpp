#include "Engine.h"
#include "vis/system.h"

#define DBG_TIMESTAMP(n) "["#n" " + std::to_string(milliseconds() - Time::startMillis) + "]"

#ifndef DBG_TIMESTAMP
#define DBG_TIMESTAMP(n) "["#n"]"
#endif

byte Debug::suppress = 0;

#define FLUSH "\n"; std::flush(std::cout)

void Debug::Message(std::string c, std::string s) {
	if (stream) *stream << DBG_TIMESTAMP(i) + c + ": " + s + FLUSH;
	if (suppress == 0)
		std::cout << DBG_TIMESTAMP(i) + c + ": " + s + FLUSH;
}
void Debug::Warning(std::string c, std::string s) {
	if (stream) *stream << DBG_TIMESTAMP(w) + c + ": " + s + FLUSH;
	if (suppress <= 1) {
#ifdef PLATFORM_WIN
		SetConsoleTextAttribute(winHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
		std::cout << "\033[33m";
#endif
		std::cout << DBG_TIMESTAMP(w) + c + ": " + s + FLUSH;
#ifdef PLATFORM_WIN
		SetConsoleTextAttribute(winHandle, winTextAttr);
#else
		std::cout << "\033[0m";
		std::flush(std::cout);
#endif
	}
}
void Debug::Error(std::string c, std::string s) {
	if (stream) *stream << DBG_TIMESTAMP(e) + c + ": " + s + FLUSH;
#ifdef PLATFORM_WIN
	SetConsoleTextAttribute(winHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
	std::cout << "\033[31m";
#endif
	std::cout << DBG_TIMESTAMP(e) + c + ": " + s + FLUSH;
#ifdef PLATFORM_WIN
	SetConsoleTextAttribute(winHandle, winTextAttr);
#else
	std::cout << "\033[0m";
	std::flush(std::cout);
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
void Debug::Init() {
	const std::string ss (VisSystem::localFd + "Log.txt");
	stream = new std::ofstream(ss.c_str());

#ifdef PLATFORM_WIN
	winHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(winHandle, &info);
	winTextAttr = info.wAttributes;
#endif

	Message("Debug", "Log Initialized");
}