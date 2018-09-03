#pragma once
#include "Engine.h"
#include "anweb.h"

class FReader {
public:
	static void Init();
	
	static bool Read(string path, FScript* scr);

	static void Refresh(FScript* scr);

protected:
	static bool ParseType(string& s, CVar* var);
};