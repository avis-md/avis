#pragma once
#include "Engine.h"
#include <Python.h>

enum class PY_VARTYPE {
	INT,
	FLOAT,
	LIST,
	PAIR,
};

struct PyVar {
public:
	string name, typeName;
	PY_VARTYPE type;
	PyVar *child1, *child2; //1 for list, (1,2) for pair
	int ival;
	float fval;

	PyObject* value;
};

class PyScript {
public:
	string name;
	std::vector<PyVar> invars, outvars;
	uint invarCnt, outvarCnt;

	string Exec();
	void SetVal(uint i, int v), SetVal(uint i, float v);
	void* Get(uint i);

	PyObject *pModule, *pFunc, *pArgs;
	std::vector<PyObject*> pRets;
};

class PyReader {
public:
	static void Init();
	
	static bool Read(string path, PyScript** scr);

protected:
	static bool ParseType(string s, PyVar* var);
};