#pragma once
#include "Engine.h"
#include "anweb.h"

class CReader {
public:
	static void Init();

	static bool Read(string path, CScript* scr);
	static void Refresh(CScript* scr);

protected:
	static string vcbatPath;
	static bool useOMP;
	static string flags1, flags2;
	static bool ParseType(string s, CVar* var);
};