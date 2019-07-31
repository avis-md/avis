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
#include "Engine.h"
#include "errorview.h"
#include "anscript_vars.h"

class AnScript;
typedef std::shared_ptr<AnScript> pAnScript;
class AnScript_I;
typedef std::shared_ptr<AnScript_I> pAnScript_I;

class AnScript {
public:
	virtual ~AnScript() {}

	struct Var {
		std::string name;
		AN_VARTYPE type, itemType;
		int dim;
		std::string typeName;
		int stride;
		enum class UI_TYPE {
			NONE,
			ENUM,
			RANGE
		} uiType;
		std::vector<std::string> enums;
		Vec2 range;

		void InitName();
	};
	enum class TYPE : byte {
		NONE,
		C,
		PYTHON,
		FORTRAN
	};
	typedef void* (*spawnerFunc)(void);
	typedef void (*callerFunc)(void*);
	typedef void (*deleterFunc)(void*);

	AnScript(TYPE t) : type(t) {}

	std::string name, path, scrPath;
	const TYPE type;
	bool isSingleton;

	bool ok = false, busy = false;
	bool allowParallel = true;
	std::mutex parallelLock;
	time_t chgtime = 0, badtime = 0;
	std::vector<ErrorView::Message> compileLog;
	int errorCount = 0;
	std::string desc = "";
	int descLines = 0;

	std::vector<Var> inputs, outputs;
	
	spawnerFunc spawner;
	callerFunc caller;
	deleterFunc deleter;

	virtual void Clear();
	virtual pAnScript_I CreateInstance() = 0;
};

class AnScript_I {
public:
	virtual ~AnScript_I();

	AnScript* parent;
	void* instance;

	union DefVal {
		short s;
		int i;
		double d;
		uint64_t data;
	};
	std::vector<DefVal> defVals;

	virtual void Init(AnScript* pr);

	virtual void* Resolve(uintptr_t offset) = 0;
	virtual int* GetDimValue(const CVar::szItem& i) = 0;

	virtual void SetInput(int i, short val) = 0;
	virtual void SetInput(int i, int val) = 0;
	virtual void SetInput(int i, double val) = 0;
	virtual void SetInput(int i, void* val, char tp, std::vector<int> szs) = 0;
	virtual void GetOutput(int i, int* val) = 0;
	
	virtual void Execute();
	virtual float GetProgress();
};

#include "dmscript.h"
#include "cc/cscript.h"
#include "py/pyscript.h"
#include "ft/fscript.h"