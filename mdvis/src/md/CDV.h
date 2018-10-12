#pragma once
#include "importer_info.h"

//CDView molecular data file parser
class CDV {
public:
	static bool Read(ParInfo* info);
	static bool ReadFrame(FrmInfo* info);
private:
	static std::vector<std::string> SplitString(std::string s, char c, bool rm);
};