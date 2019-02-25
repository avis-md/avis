#pragma once
#include "web/anweb.h"

class PyReader {
public:
	static bool initd;

	static void Init(), Deinit();
	
	static bool Read(PyScript* scr);

	static void Refresh(PyScript* scr);

protected:
	static bool ParseType(AnScript::Var& var);
};