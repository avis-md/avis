#pragma once
#include "Engine.h"
#include "errorview.h"
#include "anscript_vars.h"

class AnScript_I;

class AnScript {
public:
	virtual ~AnScript();

	struct Var {
		std::string name;
		AN_VARTYPE type, itemType;
		std::string typeName;
		enum class UI_TYPE {
			NONE,
			ENUM,
			RANGE
		} uiType;
		std::vector<std::string> enums;
		Vec2 range;
		uintptr_t offset;
	};
	enum class TYPE : byte {
		NONE,
		C,
		PYTHON,
		FORTRAN
	};

	typedef void* (*spawnerFunc)(void);
	typedef void (*callerFunc)(void*);

	std::string name, path;
	const TYPE type;

	bool ok = false, busy = false;
	time_t chgtime;
	std::vector<ErrorView::Message> compileLog;
	int errorCount;
	std::string desc;
	int descLines;

	std::vector<Var> inputs, outputs;
	
	spawnerFunc spawner;
	callerFunc caller;

	virtual bool Clear();
	virtual std::shared_ptr<AnScript_I> CreateInstance() = 0;
};

class AnScript_I {
public:
	virtual ~AnScript_I();

	AnScript* parent;
	void* instance;

	virtual void* Resolve(uintptr_t offset);

	virtual void SetInput(int i, int val) = 0;
	virtual void GetOutput(int i, int* val) = 0;
	
	virtual void Execute();
	const float GetProgress();
};

#include "dmscript.h"
#include "cc/cscript.h"