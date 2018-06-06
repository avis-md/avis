#pragma once
#include "Engine.h"
#include "anscript.h"
#include "annode.h"
#include "anbrowse.h"
#include "pyreader.h"
#include "creader.h"

class AnWeb {
public:
	static AnNode* selConnNode;
	static uint selConnId;
	static bool selConnIdIsOut, selPreClear;
	static AnScript* selScript;

	static std::vector<AnNode*> nodes;

	static bool drawFull, expanded, executing;
	static float maxScroll, scrollPos, expandPos;

	static std::thread* execThread;

	static void Insert(AnScript* scr, Vec2 pos = Vec2(100, 100));
	static void Insert(AnNode* node, Vec2 pos = Vec2(100, 100));
	static void Init(), Update(), Draw(), DrawSide(), DrawScene(), Execute(), DoExecute();
	static void Save(const string& s), Load(const string& s);
};

#include "anbrowse.h"
#include "nodes/node_inputs.h"
#include "nodes/node_plot.h"
#include "nodes/node_volume.h"