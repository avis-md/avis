#pragma once
#include "parloader.h"

class XYZ {
public:
	static bool Read(ParInfo* info);
	static bool ReadTrj(TrjInfo* info);
};