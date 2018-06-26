#pragma once
#include "ChokoLait.h"
#include "Particles.h"
#include "utils/dylib.h"

struct ParInfo {
	const char* path; //IN
	byte nameSz; //IN
	float* progress; //IN
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
	float* progress; //IN
	byte padding[3];
	uint16_t frames;
	float** poss;
	float** vels;
	char error[100];
};

struct ParImporter {
	typedef bool(*loadsig)(ParInfo*);
	typedef bool(*loadtrjsig)(TrjInfo*);
	string name, sig;
	DyLib* lib;
	std::vector<std::pair<std::vector<string>, loadsig>> funcs;
	std::vector<std::pair<std::vector<string>, loadtrjsig>> trjFuncs;
};

class ParLoader {
public:
	static void Init(), Scan();

	static int impId, funcId;
	static ParImporter* customImp;
	static bool loadAsTrj, additive;
	static int maxframes;

	static std::vector<ParImporter*> importers;
	
	static bool showDialog, busy, fault;
	static bool parDirty, trjDirty;
	static float loadProgress;
	static std::vector<string> droppedFiles;

	static bool _showImp;
	static float _impPos, _impScr;

	static void DoOpen();
	static void DoOpenAnim();

	static void DrawOpenDialog();

	static bool OnDropFile(int i, const char** c);
	static void FindImpId(bool force = false);
};