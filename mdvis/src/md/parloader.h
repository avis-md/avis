#pragma once
#include "ChokoLait.h"
#include "Particles.h"

struct ParInfo {
	char* path; //IN
	byte nameSz; //IN
	byte padding[3];
	uint32_t num;
	char* resname;
	char* name;
	uint16_t* type;
	uint32_t resId;
	float* pos;
	float* vel;
};

class ParLoader {
public:
	static void Init();

	static bool loadAsTrj, additive;
	static int maxframes;

	static std::vector<std::pair<std::vector<string>, string>> importers;
	
	static bool showDialog;
	static std::vector<string> droppedFiles;


	static bool Open(const char* path);
	static bool OpenAnim(uint num, const char** paths);

	static void DrawOpenDialog();

	static bool OnDropFile(int i, const char** c);
};