#pragma once
#include "Engine.h"
#include <Python.h>
#include "errorview.h"
#ifndef IS_ANSERVER
#include "utils/dylib.h"
#endif

enum class AN_SCRTYPE : byte {
	NONE,
	C,
	PYTHON,
	FORTRAN
};

enum class AN_VARTYPE : byte {
	INT,
	DOUBLE,
	LIST,
	ID_RSL,
	ID_RES,
	ID_PAR,
};

union _VarVal {
	int i;
	double d;
	struct arr {
		void* p;
		std::vector<char> data;
	} arr;

	_VarVal() {
		memset(this, 0, sizeof(_VarVal));
	}
	~_VarVal() {}
	_VarVal(const _VarVal& o) {
		memcpy(this, &o, sizeof(_VarVal));
	}
	_VarVal& operator=(const _VarVal& o) {
		memcpy(this, &o, sizeof(_VarVal));
		return *this;
	}
};

struct VarVal {
	_VarVal val;
	std::vector<int> dims;
};

class AnScript {
public:
	string name, path, libpath;
	time_t chgtime;
	AN_SCRTYPE type;
	std::vector<std::pair<string, string>> invars, outvars;
	void* progress = 0;

	std::vector<ErrorView::Message> compileLog;
	int errorCount = 0;

	string desc = "";
	int descLines = 0;
	bool ok = false;

	static int StrideOf(char c);

	virtual void Clear();
	virtual string Exec() = 0;
protected:
	AnScript(AN_SCRTYPE t) : type(t) {}
};

class DmScript : public AnScript {
public:
	DmScript(string nm) : AnScript(AN_SCRTYPE::NONE) {
		ok = true;
		name = nm;
	}

	void Clear() override {}
	string Exec() override { return ""; }
};

struct PyVar {
public:
	string name, typeName;
	AN_VARTYPE type;
	PyObject* value;
	int dim, stride;
};

class PyScript : public AnScript {
public:
	PyScript() : AnScript(AN_SCRTYPE::PYTHON), pModule(nullptr) {}

	std::vector<PyVar> _invars, _outvars;
	
	static void InitLog();
	static string GetLog();
	static void ClearLog();

	static PyObject* mainModule, *logCatcher, *emptyString;

	void Clear() override;
	string Exec() override;
	void Set(uint i, int v), Set(uint i, double v), Set(uint i, PyObject* v);

	PyObject* pModule, *pFunc, *pArgl;
	std::vector<PyObject*> pArgs, pRets;

	static std::unordered_map<string, PyScript*> allScrs;
};

struct CVar {
public:
	CVar() : value(0) {}

	string name, typeName;
	AN_VARTYPE type;

	VarVal data;
	void* value;
	std::vector<string> dimNames;
	std::vector<int*> dimVals;
	int stride;

	void Write(std::ofstream& strm);
	void Read(std::ifstream& strm);
};

typedef void(*emptyFunc)();
typedef char* (*wrapFunc)();

class CScript : public AnScript {
public:
	CScript() : AnScript(AN_SCRTYPE::C) {}

	std::vector<CVar> _invars, _outvars;

	void Clear() override;
	string Exec() override;
	
	template<typename T> void Set(int i, T& val) {
		*((T*)_invars[i].value) = val;
	}
	
	DyLib* lib;
	
	emptyFunc funcLoc;
	wrapFunc wFuncLoc;

	static std::unordered_map<string, CScript*> allScrs;
};

class FScript : public AnScript {
public:
	FScript() : AnScript(AN_SCRTYPE::FORTRAN) {}

	std::vector<CVar> _invars, _outvars;
	std::vector<emptyFunc> _inarr_pre, _outarr_post;

	int32_t** arr_shapeloc;
	void** arr_dataloc;
	
	void Clear() override;
	string Exec() override;

	template<typename T> void Set(int i, T& val) {
		*((T*)_invars[i].value) = val;
	}
	
	DyLib* lib;
	
	wrapFunc funcLoc;

	static std::unordered_map<string, FScript*> allScrs;
};