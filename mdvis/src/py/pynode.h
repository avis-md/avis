#pragma once
#include "pyreader.h"

enum class PYNODE_OP {
	NONE,
	LEFT,
	RIGHT,
	REMOVE
};

class PyNode {
public:
	PyNode(PyScript* scr);

	PyScript* script;
	bool selected;
	Vec2 pos;
	static float width;
	string title;
	Vec3 titleCol = Vec3(0.35f, 0.35f, 0.35f);

	bool expanded = true;
	bool canTile = false;
	PYNODE_OP op;
	
	static Font* font;

	std::vector<PyNode*> inputR, outputR;
	std::vector<std::pair<PyVar, uint>> inputV, outputV;

	static void Init();

	bool Select();
	virtual Vec2 DrawConn();
	virtual void Draw();
	virtual float DrawSide();
	void DrawToolbar();
	virtual void Execute();
	void ConnectTo(uint id, PyNode* tar, uint tarId); //out -> in
	
protected:
	static Texture* tex_circle_open, *tex_circle_conn;
};

#include "nodes/pynode_plot.h"
#include "nodes/pynode_inputs.h"