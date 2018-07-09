#pragma once
#include "SceneObjects.h"

enum SCR_VARTYPE : byte {
	SCR_VAR_UNDEF = 0,
	SCR_VAR_INT, SCR_VAR_UINT,
	SCR_VAR_FLOAT,
	SCR_VAR_V2, SCR_VAR_V3, SCR_VAR_V4,
	SCR_VAR_STRING,
	SCR_VAR_ASSREF = 0x20,
	SCR_VAR_COMPREF = 0x30,
	SCR_VAR_SCR = 0xf0,
	SCR_VAR_COMMENT = 0xff
};

#ifdef IS_EDITOR
class SCR_VARVALS {
public:
	SCR_VARTYPE type;
	string desc;
	void* val;
};
#endif

class SceneScript : public Component {
public:
	~SceneScript();

	virtual void Start() {}
	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void Paint() {}

	friend void Deserialize(std::ifstream& stream, SceneObject* obj);
	_allowshared(SceneScript);
protected:
	SceneScript() : Component("", COMP_SCR, DRAWORDER_NONE) {}
};
