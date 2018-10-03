#pragma once
#include "parloader.h"

//CDView molecular data file parser
class CDV {
public:
	static bool Read(ParInfo* info);
	static bool ReadFrame(FrmInfo* info);
};