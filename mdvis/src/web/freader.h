#pragma once
#include "Engine.h"
#include "anweb.h"

class FReader {
public:
	static void Init();
	
	static bool Read(FScript* scr);

	static void Refresh(FScript* scr);

protected:
	static bool ParseType(string& s, CVar* var);

	struct typestring {
		string type, name, dims;

		typestring(string a, string b, string c) : type(a), name(b), dims(c) {}
	};
	static void GenArrIO(string path, string name, std::vector<typestring> invars, std::vector<string> outvars);
};