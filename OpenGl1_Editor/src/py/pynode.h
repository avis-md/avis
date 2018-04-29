#pragma once
#include "pyreader.h"

class PyNode {
public:
	PyNode(PyScript* scr);

	PyScript* script;
	bool selected;
	Vec2 pos;
	static float width;

	bool expanded = true;
	bool canTile = false;

	static Font* font;

	std::vector<PyNode*> inputR, outputR;
	std::vector<std::pair<PyVar, uint>> inputV, outputV;

	static void Init();

	bool Select();
	virtual Vec2 DrawConn();
	virtual void Draw();
	virtual float DrawSide();
	virtual void Execute();
	void ConnectTo(uint id, PyNode* tar, uint tarId); //out -> in
	
protected:
	static Texture* tex_circle_open, *tex_circle_conn;
};

#include "nodes/pynode_plot.h"
#include "nodes/pynode_inputs.h"