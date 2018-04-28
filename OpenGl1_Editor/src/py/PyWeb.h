#pragma once
#include "Engine.h"
#include "pynode.h"

class PyWeb {
public:
	static void Insert(PyScript* scr, Vec2 pos = Vec2(100, 100));
	static void Insert(PyNode* node, Vec2 pos = Vec2(100, 100));
	static void Update(), Draw(), Execute();

	static PyNode* selConnNode;
	static uint selConnId;
	static bool selConnIdIsOut, selPreClear;

	static std::vector<PyNode*> nodes;
};