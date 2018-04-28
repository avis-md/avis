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
	void Draw(), DrawConn();

protected:
	static Texture* tex_circle_open, *tex_circle_conn;
};

class PyGraph {
public:
	std::vector<PyNode*> nodes;
};