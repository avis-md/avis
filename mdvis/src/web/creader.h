#pragma once
#include "Engine.h"
#include "anweb.h"

class CReader {
public:
	static void Init();

	static bool Read(string path, CScript** scr);

protected:
#ifdef PLATFORM_WIN
	static string vcbatPath;
#endif
	static bool ParseType(string s, CVar* var);
};