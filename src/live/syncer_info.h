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


struct SyncInfo {
	uint32_t num; //init only
	byte namesz; //IN
	char* resname; //init only
	char* name; //init only
	uint16_t* type; //init only
	uint16_t* resId; //init only
	float bounds[6]; //init only
	bool fill;
	float* pos;
	float* vel;
};