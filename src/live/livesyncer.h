#pragma once
#include "Engine.h"
#include "utils/dylib.h"
#define NO_EXPORT_IMP
#include "syncer_info.h"

struct LiveRunner {
	typedef bool(*initSig)(SyncInfo*);
	typedef bool(*loopSig)(SyncInfo*);
	std::string name, path;
	std::string initNm, loopNm;
	DyLib* lib;
	initSig initFunc;
	loopSig loopFunc;
};

class LiveSyncer {
public:
	static enum LIVE_STATUS : byte {
		MENU,
		IDLE,
		PAUSE,
		LOOP,
		FAIL
	} status;
	static std::vector<LiveRunner*> runners;
	static LiveRunner* activeRunner;
	static SyncInfo info;

	static void Init(uint i), Start(), Update(), Pause(), Stop();
	
	static bool expanded;
	static float expandPos;
	static void DrawSide();

	static std::thread* runThread;
	static void DoRun();

	static uint tarFrm;
	static bool applyFrm;
};