#pragma once
#include "ChokoLait.h"
#include "Particles.h"
#include "utils/dylib.h"

struct ParInfo {
	const char* path; //IN
	byte nameSz; //IN
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
	byte padding[5];
	uint16_t frames;
	float** poss;
	float** vels;
	char error[100];
};

struct ParImporter {
	typedef bool(*loadsig)(ParInfo*);
	typedef bool(*loadtrjsig)(ParInfo*);
	string name, sig;
	DyLib* lib;
	std::vector<std::pair<std::vector<string>, loadsig>> funcs;
	std::vector<std::pair<std::vector<string>, loadtrjsig>> trjFuncs;
};

class ParLoader {
public:
	static void Init();

	static int impId, funcId;
	static ParImporter* customImp;
	static bool loadAsTrj, additive;
	static int maxframes;

	static std::vector<ParImporter*> importers;
	
	static bool showDialog;
	static std::vector<string> droppedFiles;

	static bool _showImp;
	static float _impPos, _impScr;

	static bool DoOpen();
	static bool DoOpenAnim();

	static void DrawOpenDialog();

	static bool OnDropFile(int i, const char** c);
};