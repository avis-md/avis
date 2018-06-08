#include "Engine.h"

byte Debug::suppress = 0;

std::ofstream* Debug::stream = nullptr;

void Debug::Message(string c, string s) {
	if (suppress > 0) return;
	if (stream) *stream << "[i]" << c << ": " << s << std::endl;
	std::cout << "[i]" << c << ": " << s << std::endl;
}
void Debug::Warning(string c, string s) {
	if (suppress > 1) return;
	if (stream) *stream << "[w]" << c << ": " << s << std::endl;
	std::cout << "[w]" << c << ": " << s << std::endl;
}
void Debug::Error(string c, string s) {
	if (stream) *stream << "[e]" << c << ": " << s << std::endl;
	std::cout << "[e]" << c << ": " << s << std::endl;
	abort();
}

uint Debug::StackTrace(uint count, void** frames) {
	uint c;
	c = backtrace(frames, count);
	return c;
}
void Debug::Init(string s) {
	string ss = s + "/Ansrv_log.txt";
	stream = new std::ofstream(ss.c_str(), std::ios::out | std::ios::trunc);
	Message("Debug", "Log Initialized");
}