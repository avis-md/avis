#pragma once
#include "Engine.h"
#include <Python.h>

enum class PY_VARTYPE {
	INT,
	FLOAT,
	LIST,
	PAIR
};

struct PyVar {
public:
	string name, typeName;
	PY_VARTYPE type;
	PyVar *child1, *child2; //1 for list, (1,2) for pair
	//union {
		int ival;
		float fval;
		std::vector<void*> lval;
		std::pair<void*, void*> pval;
	//};
};

class PyScript {
public:
	string name;
	std::vector<PyVar> invars, outvars;
	uint invarCnt, outvarCnt;

	void Exec();

	PyObject *pModule, *pFunc, *pArgs;
	std::vector<PyObject*> pRets;
};

class PyReader {
public:
	static void Init() {
		Py_Initialize();
	}

	static bool Read(string path, PyScript** scr);

protected:
	static bool ParseType(string s, PyVar* var);
};