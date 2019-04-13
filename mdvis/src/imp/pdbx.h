#pragma once
#include "importer_info.h"

class PDBx {
public:
	static bool Read(ParInfo* info);
	static bool ReadFrm(FrmInfo* info);
};
