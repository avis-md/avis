// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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
	std::vector<int> dims;
	
	VarVal() = default;
	VarVal(const VarVal& vv) {
		val = vv.val;
		arr = vv.arr;
		dims = vv.dims;
		if (arr.size() > 0) val.p = arr.data();
		if (vv.pval == &vv.val) pval = &val;
		else pval = vv.pval;
	}
	VarVal& operator= (const VarVal& vv) {
		val = vv.val;
		arr = vv.arr;
		dims = vv.dims;
		if (arr.size() > 0) val.p = arr.data();
		if (vv.pval == &vv.val) pval = &val;
		else pval = vv.pval;
		return *this;
	}
};

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

struct PyVar {
	std::string name;
	std::vector<int> szs;
};