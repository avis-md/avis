#pragma once
#include "Engine.h"
#include <Python.h>

enum class AN_SCRTYPE {
	C,
	PYTHON,
	FORTRAN
};

enum class AN_VARTYPE {
	INT,
	FLOAT,
	LIST
};

class AnScript {
public:
	string name;
	AN_SCRTYPE type;
	std::vector<std::pair<string, string>> invars, outvars;

	virtual string Exec() = 0;
	virtual void Set(uint i, int v) = 0, Set(uint i, float v) = 0, Set(uint i, void* v) = 0;
	virtual void* Get(uint i) = 0;
};

struct PyVar {
public:
	string name, typeName;
	PY_VARTYPE type;
	PyVar *child1, *child2;

	PyObject* value;
};

class PyScript : public AnScript {
public:
	std::vector<PyVar> _invars, _outvars;

	string Exec() override;
	void Set(uint i, int v) override, Set(uint i, float v) override, Set(uint i, void* v) override;
	void* Get(uint i) override;

	PyObject *pModule, *pFunc, *pArgs;
	std::vector<PyObject*> pRets;
};

class CScript : public AnScript {
	string Exec() override;
	void Set(uint i, int v) override, Set(uint i, float v) override, Set(uint i, void* v) override;
	void* Get(uint i) override;

	void *pModule, *pFunc, *pArgs;
	std::vector<PyObject*> pRets;
};