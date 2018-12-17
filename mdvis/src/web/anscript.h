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
	SHORT,
	INT,
	DOUBLE,
	LIST,
};

union _VarVal {
	short s;
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

struct VarOpt {
	enum {
		NONE,
		ENUM,
		RANGE
	} type;
	std::vector<std::string> enums;
	Vec2 range;
};

struct CVar;

class AnScript {
public:
	std::string name, path, libpath;
	time_t chgtime;
	const AN_SCRTYPE type;
	std::vector<std::pair<std::string, std::string>> invars, outvars;
	std::vector<VarOpt> invaropts;
	double* progress = 0;

	std::vector<ErrorView::Message> compileLog;
	int errorCount = 0;

	std::string desc = "";
	int descLines = 0;
	bool ok = false, busy = false;

	static int StrideOf(char c);

	void AddInput(std::string nm, std::string tp);
	void AddInput(const CVar& cv);
	void AddOutput(std::string nm, std::string tp);
	void AddOutput(const CVar& cv);

	virtual bool Clear();
	virtual std::string Exec() = 0;
protected:
	AnScript(AN_SCRTYPE t) : type(t) {}
};

class DmScript : public AnScript {
public:
	DmScript(std::string nm) : AnScript(AN_SCRTYPE::NONE) {
		ok = true;
		name = nm;
	}

	bool Clear() override { return true; }
	std::string Exec() override { return ""; }
};

struct PyVar {
public:
	~PyVar();
	std::string name, typeName;
	AN_VARTYPE type;
	PyObject* value = nullptr;
	int dim, stride;
};

class PyScript : public AnScript {
public:
	PyScript() : AnScript(AN_SCRTYPE::PYTHON), pModule(0), pFunc(0) {}

	std::vector<PyVar> _invars, _outvars;
	
	static void InitLog();
	static std::string GetLog();
	static void ClearLog();

	static PyObject* mainModule, *logCatcher, *emptyString;

	bool Clear() override;
	std::string Exec() override;
	void Set(uint i, int v), Set(uint i, double v), Set(uint i, PyObject* v);

	PyObject* pModule, *pFunc;

	static std::unordered_map<std::string, PyScript*> allScrs;
};

struct CVar {
public:
	CVar() : value(0) {}
	CVar(std::string nm, AN_VARTYPE tp);
	CVar(std::string nm, char tp, int dim, std::initializer_list<int*> szs, std::initializer_list<int> defszs = {});
	CVar(const CVar&);
	CVar& operator= (const CVar&);

	std::string name, typeName;
	AN_VARTYPE type;

	VarVal data;
	void* value;
	std::vector<std::string> dimNames;
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

	bool Clear() override;
	std::string Exec() override;
	
	template<typename T> void Set(int i, T& val) {
		*((T*)_invars[i].value) = val;
	}
	
	DyLib lib;
	
	emptyFunc funcLoc;
	wrapFunc wFuncLoc;

	static std::unordered_map<std::string, CScript*> allScrs;
};

class FScript : public AnScript {
public:
	FScript() : AnScript(AN_SCRTYPE::FORTRAN) {}

	std::vector<CVar> _invars, _outvars;
	std::vector<emptyFunc> _inarr_pre, _outarr_post;
	int pre, post;

	int32_t** arr_in_shapeloc;
	void** arr_in_dataloc;
	int32_t** arr_out_shapeloc;
	void** arr_out_dataloc;

	bool Clear() override;
	std::string Exec() override;

	template<typename T> void Set(int i, T& val) {
		*((T*)_invars[i].value) = val;
	}
	
	DyLib lib;
	
	wrapFunc funcLoc;

	static std::unordered_map<std::string, FScript*> allScrs;
};