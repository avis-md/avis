#pragma once
#include <stdint.h>

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif
typedef unsigned char byte;

struct ParInfo {
	const char* path; //IN
	byte nameSz; //IN
	float* progress;
	byte padding[6];
	uint32_t num;
	char* resname;
	char* name;
	uint16_t* type; //H\0 if hydrogen
	uint16_t* resId;
	float* pos;
	float* vel;
	float bounds[3];
	char error[100];
};

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