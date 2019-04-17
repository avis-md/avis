#pragma once
#include "syncer_info.h"

class LJ256 {
public:
	static bool Init(SyncInfo* info);
	static bool Loop(SyncInfo* info);
};