#pragma once
#include "pyreader.h"

class PyNode {
public:
	PyNode(PyScript* scr);
	
	PyScript* script;

	static Font* font;

	std::vector<PyNode*> inputR, outputR;
	std::vector<std::pair<PyVar, uint>> inputV, outputV;

	static void Init();
	void Draw(Vec2 pos);

protected:
	static Texture* tex_circle_open, *tex_circle_conn;
};

class PyGraph {
public:
	std::vector<PyNode*> nodes;
};