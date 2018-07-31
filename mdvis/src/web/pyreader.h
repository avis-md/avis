#pragma once
#include "Engine.h"
#include "anweb.h"

class PyReader {
public:
	static void Init();
	
	static bool Read(string path, PyScript* scr);

	static void Refresh(PyScript* scr);

protected:
	static bool ParseType(string s, PyVar* var);
};