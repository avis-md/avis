#pragma once
#include "ChokoLait.h"
#include "pynode.h"
#include "md/ParMenu.h"

class PyWeb {
public:
	static void Insert(PyScript* scr, Vec2 pos = Vec2(100, 100));
	static void Insert(PyNode* node, Vec2 pos = Vec2(100, 100));
	static void Init(), Update(), Draw(), DrawSide(), Execute(), DoExecute();
	static void blitfunc();
	
	static PyNode* selConnNode;
	static uint selConnId;
	static bool selConnIdIsOut, selPreClear;
	static PyScript* selScript;
	
	static std::vector<PyNode*> nodes;
	
	static bool drawFull, expanded, executing;
	static float maxScroll, scrollPos, expandPos;
	
	static uint hlId1, hlId2;
	static GLuint selHlProgram, selHlRProgram, colorerProgram;
	static GLint selHlLocs[4], selHlRLocs[5], colorerLocs[4];

	static std::thread* execThread;
};

#include "pyreader.h"
#include "pybrowse.h"
#include "nodes/pynode_inputs.h"
#include "nodes/pynode_plot.h"