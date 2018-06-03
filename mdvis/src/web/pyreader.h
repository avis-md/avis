#pragma once
#include "Engine.h"
#include <Python.h>
#include "anweb.h"

enum class PY_VARTYPE {
	INT,
	FLOAT,
	LIST,
	PAIR,
};

class PyReader {
public:
	static void Init();
	
	static bool Read(string path, PyScript** scr);

protected:
	static bool ParseType(string s, PyVar* var);
};