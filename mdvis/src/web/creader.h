#pragma once
#include "Engine.h"
#include "anweb.h"

class CReader {
public:
	static void Init();

	static bool Read(string path, CScript** scr);

protected:
	static string vcbatPath;
	static bool ParseType(string s, CVar* var);
};