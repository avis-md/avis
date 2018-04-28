#pragma once
#include "pyreader.h"

class PyNode {
public:
	PyNode(PyScript* scr);

	PyScript* script;
	bool selected;
	Vec2 pos;

	static Font* font;

	std::vector<PyNode*> inputR, outputR;
	std::vector<std::pair<PyVar, uint>> inputV, outputV;

	static void Init();

	bool Select();
	virtual void Draw(), DrawConn();
	virtual void Execute();

protected:
	static Texture* tex_circle_open, *tex_circle_conn;
};

#include "nodes/pynode_plot.h"