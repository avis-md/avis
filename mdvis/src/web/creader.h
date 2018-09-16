#pragma once
#include "Engine.h"
#include "anweb.h"

class CReader {
public:
	static void Init();

	static void Compile(string path);
	static bool Read(CScript* scr);
	static void Refresh(CScript* scr);

	friend class CScript;
	friend class FReader;
protected:
	static string gpp;
	static string vcbatPath, mingwPath;
	static bool useMsvc, useOMP;
	static string flags1, flags2;
	static bool ParseType(string s, CVar* var);
};