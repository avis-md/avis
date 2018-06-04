#pragma once
#include "anscript.h"

enum class ANNODE_OP {
	NONE,
	LEFT,
	RIGHT,
	REMOVE
};

union AnVarBase {
	int i;
	float f;
};

class AnNode {
public:
	AnScript* script;
	bool selected;
	Vec2 pos;
	static float width;
	string title;
	Vec3 titleCol = Vec3(0.35f, 0.35f, 0.35f);

	bool expanded = true;
	bool canTile = false;
	ANNODE_OP op;

	static Font* font;

	std::vector<std::pair<AnNode*, uint>> inputR, outputR;
	std::vector<union AnVarBase> inputVDef;
	std::vector<CVar> conV;

	static void Init();

	bool Select();
	virtual Vec2 DrawConn();
	virtual void Draw();
	virtual float DrawSide();
	void DrawToolbar();
	virtual void Execute() = 0;
	void ConnectTo(uint id, AnNode* tar, uint tarId); //out -> in

protected:
	AnNode(AnScript* scr);
	
	static Texture* tex_circle_open, *tex_circle_conn;
};

class PyNode : public AnNode {
public:
	PyNode(PyScript*);
	
	std::vector<PyVar> inputV, outputV;
	
	void Execute() override;
};

class CNode : public AnNode {
public:
	CNode(CScript*);

	std::vector<void*> inputV, outputV;

	void Execute() override;
};