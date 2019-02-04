#pragma once
#include "anweb.h"

enum class AN_VARTYPE : byte {
	NONE,
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

/*
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
*/

struct CVar {
	struct szItem {
		bool useOffset;
		union {
			uintptr_t offset;
			int size;
		};
	};

	uintptr_t offset;
	std::vector<szItem> szOffsets;
};