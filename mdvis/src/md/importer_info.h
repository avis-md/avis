#pragma once
#include <stdint.h>

#ifndef NO_EXPORT_IMP
#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif
typedef unsigned char byte;
typedef unsigned int uint;
#endif

#ifdef _WIN32
#include <string>
#include <locale>
#include <codecvt>
inline std::wstring PATH(const std::string& s) {
	return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes(s);
}
#else
#define PATH(s) s
#endif

struct TrjInfo {
	const char* first; //IN
	uint32_t parNum; //IN
	uint16_t maxFrames; //IN
	uint16_t frameSkip; //IN
	float progress;
	byte padding[3];
	uint16_t frames;
	double** poss;
	double** vels;
	char error[100];
};

struct ParInfo {
	const char* path; //IN
	byte nameSz; //IN
	float progress; //OPT
	byte padding[6];
	uint32_t num;
	char* resname;
	char* name;
	uint16_t* type; //H\0 if hydrogen
	uint16_t* resId;
	double* pos;
	double* vel; //OPT
	struct ProSec{
		enum TYPE { HELIX, SHEET } type;
		uint16_t resSt, resEd;
		byte helixPitch;
	}* secStructs; //OPT
	uint16_t secStructNum;
	float bounds[6];
	TrjInfo trajectory; //OPT
	char error[100]; //OPT
};