#pragma once
#include "Engine.h"
#include "anweb.h"

class CReader {
public:
	static void Init();

	static void Compile(std::string path);
	static bool Read(CScript* scr);
	static void Refresh(CScript* scr);

	friend class CScript;
	friend class FReader;
protected:
	static std::string gpp;
	static std::string vcbatPath, mingwPath;
	static bool useMsvc, useOMP, useOMP2;
	static std::string flags1, flags2;
	static AN_VARTYPE ParseType(const std::string& s);

	struct VarInfo {
		std::string name;
		AN_VARTYPE type, itemType;
		std::string error;
	};
	static VarInfo ParseVar(std::string s);
};