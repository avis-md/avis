#include "Engine.h"

byte Debug::suppress = 0;

void Debug::Message(string c, string s) {
	if (suppress > 0) return;
	if (stream) *stream << "[i]" << c << ": " << s << std::endl;
#ifdef PLATFORM_ADR
	__android_log_print(ANDROID_LOG_INFO, "ChokoLait", &s[0]);
#else
	std::cout << "[i]" << c << ": " << s << std::endl;
#endif
}
void Debug::Warning(string c, string s) {
	if (suppress > 1) return;
	if (stream) *stream << "[w]" << c << ": " << s << std::endl;
#ifdef PLATFORM_ADR
	__android_log_print(ANDROID_LOG_WARN, "ChokoLait", &s[0]);
#endif
	std::cout << "[w]" << c << ": " << s << std::endl;
}
void Debug::Error(string c, string s) {
	if (stream) *stream << "[e]" << c << ": " << s << std::endl;
#ifdef PLATFORM_ADR
	__android_log_print(ANDROID_LOG_ERROR, "ChokoLait", &s[0]);
#endif
	std::cout << "[e]" << c << ": " << s << std::endl;
#ifdef PLATFORM_WIN
	__debugbreak();
#endif
	abort();
}

uint Debug::StackTrace(uint count, void** frames) {
	uint c;
#ifdef PLATFORM_WIN
	c = CaptureStackBackTrace(0, count, frames, NULL);
#else
	c = backtrace(frames, count);
#endif
	return c;
}

std::ofstream* Debug::stream = nullptr;
#ifdef PLATFORM_WIN
	HANDLE Debug::winHandle = 0;
#endif
void Debug::Init(string s) {
	string ss = s + "/Log.txt";
	stream = new std::ofstream(ss.c_str(), std::ios::out | std::ios::trunc);
	Message("Debug", "Log Initialized");
}