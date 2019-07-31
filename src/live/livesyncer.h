// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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