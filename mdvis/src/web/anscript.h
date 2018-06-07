#pragma once
#include "Engine.h"
#include <Python.h>
#include "utils/dylib.h"

enum class AN_SCRTYPE {
	NONE,
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
protected:
	AnScript(AN_SCRTYPE t) : type(t) {}
};

class DmScript : public AnScript {
public:
	DmScript() : AnScript(AN_SCRTYPE::NONE) {}

	string Exec() override { return ""; }
	void Set(uint i, int v) override, Set(uint i, float v) override, Set(uint i, void* v) override;
	void* Get(uint i) override;
};

struct PyVar {
public:
	string name, typeName;
	AN_VARTYPE type;
	PyObject* value;
	int dim;
};

class PyScript : public AnScript {
public:
	PyScript() : AnScript(AN_SCRTYPE::PYTHON) {}

	std::vector<PyVar> _invars, _outvars;
	
	string Exec() override;
	void Set(uint i, int v) override, Set(uint i, float v) override, Set(uint i, void* v) override;
	void* Get(uint i) override;

	PyObject* pModule, *pFunc, *pArgs;
	std::vector<PyObject*> pRets;

	static std::unordered_map<string, PyScript*> allScrs;
};

struct CVar {
public:
	CVar() : value(0) {}

	string name, typeName;
	AN_VARTYPE type;

	void* value; //same as pRets
	std::vector<string> dimNames;
	std::vector<int*> dimVals;
};

class CScript : public AnScript {
public:
	CScript() : AnScript(AN_SCRTYPE::C) {}

	std::vector<CVar> _invars, _outvars;

	string Exec() override;
	void Set(uint i, int v) override, Set(uint i, float v) override, Set(uint i, void* v) override;
	void* Get(uint i) override;
	DyLib* lib;
	
	typedef void (*emptyFunc)();
	emptyFunc funcLoc;

	static std::unordered_map<string, CScript*> allScrs;
};