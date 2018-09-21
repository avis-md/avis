#pragma once
#include "Engine.h"
#include "anweb.h"

class FReader {
public:
	static void Init();
	
	static bool Read(FScript* scr);

	static void Refresh(FScript* scr);

protected:
	static bool ParseType(std::string& s, CVar* var);

	struct typestring {
		std::string type, name, dims;

		typestring(std::string a, std::string b, std::string c) : type(a), name(b), dims(c) {}
	};
	static void GenArrIO(std::string path, std::string name, std::vector<typestring> invars, std::vector<std::string> outvars);
};