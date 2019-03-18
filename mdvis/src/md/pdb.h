#pragma once
#include "importer_info.h"

class PDB {
public:
	static bool Read(ParInfo* info);
	static bool ReadFrm(FrmInfo* info);
};