#pragma once
#include "anweb.h"

enum class AN_VARTYPE : byte {
	NONE,
	SHORT,
	INT,
	DOUBLE,
	LIST,
	ANY
};

const std::string AN_VARTYPE_STRS[] = {
	"",
	"short",
	"int",
	"double",
	"list(##)",
	"*"
};

const int AN_VARTYPE_STRIDES[] = {
	0,
	sizeof(short),
	sizeof(int),
	sizeof(double),
	-1,
	0
};

struct VarVal {
	union {
		short s;
		int i;
		double d;
		void* p;
	} val;
	void* pval;
	std::vector<char> arr;
	
	VarVal() = default;
	VarVal(const VarVal& vv) {
		val = vv.val;
		arr = vv.arr;
		if (arr.size() > 0) val.p = arr.data();
		if (vv.pval == &vv.val) pval = &val;
		else pval = vv.pval;
	}
	VarVal& operator= (const VarVal& vv) {
		val = vv.val;
		arr = vv.arr;
		if (arr.size() > 0) val.p = arr.data();
		if (vv.pval == &vv.val) pval = &val;
		else pval = vv.pval;
		return *this;
	}
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
		szItem() = default;
		szItem(void* off) : useOffset(true), offset((uintptr_t)off) {}
		szItem(int val) : useOffset(false), size(val) {}
	};

	uintptr_t offset;
	std::vector<szItem> szOffsets;
};