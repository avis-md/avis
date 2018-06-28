#pragma once
#include "ChokoLait.h"
#include "Particles.h"
#include "utils/dylib.h"
#define NO_EXPORT_IMP
#include "importer_info.h"

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
	static bool useConn, useConnCache, hasConnCache, oldConnCache, ovwConnCache;
	static string connCachePath;

	static std::vector<ParImporter*> importers;
	
	static bool showDialog, busy, fault;
	static bool parDirty, trjDirty;
	static float* loadProgress;
	static string loadName;
	static std::vector<string> droppedFiles;

	static bool _showImp;
	static float _impPos, _impScr;

	static void DoOpen();
	static void DoOpenAnim();

	static void DrawOpenDialog();

	static bool OnDropFile(int i, const char** c);
	static void FindImpId(bool force = false);
};