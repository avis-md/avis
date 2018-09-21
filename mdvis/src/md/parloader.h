#pragma once
#include "ChokoLait.h"
#include "Particles.h"
#include "utils/dylib.h"
#define NO_EXPORT_IMP
#include "importer_info.h"

struct ParImporter {
	typedef bool(*loadsig)(ParInfo*);
	typedef bool(*loadtrjsig)(TrjInfo*);
	std::string name, sig;
	DyLib* lib;
	std::vector<std::pair<std::vector<std::string>, loadsig>> funcs;
	std::vector<std::pair<std::vector<std::string>, loadtrjsig>> trjFuncs;
};

class ParLoader {
public:
	static void Init(), Scan();

	static int impId, funcId;
	static ParImporter* customImp;
	static bool loadAsTrj, additive;
	static uint frameskip;
	static int maxframes;
	static bool useConn, useConnCache, hasConnCache, oldConnCache, ovwConnCache;
	static std::string connCachePath;

	static std::vector<ParImporter*> importers;
	static std::vector<std::string> exts;
	
	static bool showDialog, busy, fault, directLoad;
	static bool parDirty, trjDirty;
	static float* loadProgress, *loadProgress2;
	static uint16_t* loadFrames;
	static std::string loadName;
	static std::vector<std::string> droppedFiles;

	static bool _showImp;
	static float _impPos, _impScr;

	static void DoOpen();
	static void DoOpenAnim();

	static void DrawOpenDialog();

	static bool OnDropFile(int i, const char** c);
	static void OnOpenFile(const std::vector<std::string>& files);
	static void FindImpId(bool force = false);
	static uint FindNextOff(std::string path);
};