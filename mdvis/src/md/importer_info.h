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

struct TrjInfo {
	const char* first; //IN
	uint32_t parNum; //IN
	uint16_t maxFrames; //IN
	uint16_t frameSkip; //IN
	float* progress;
	byte padding[3];
	uint16_t frames;
	float** poss;
	float** vels;
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
	float* pos;
	float* vel; //OPT
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