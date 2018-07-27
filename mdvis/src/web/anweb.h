#pragma once
#include "Engine.h"
#include "anscript.h"
#include "annode.h"
#include "anbrowse.h"
#include "pyreader.h"
#include "creader.h"

enum AN_NODE_MISC {
	PLOT,
	VOLUME
};
const string AN_NODE_MISC_NAMES[] = { "Plot graph", "Volume output" };

class AnWeb {
public:
	static AnNode* selConnNode;
	static uint selConnId;
	static bool selConnIdIsOut, selPreClear;
	static AnScript* selScript;
	static byte selSpNode;

	static string activeFile;
	static std::vector<AnNode*> nodes;

	static bool drawFull, expanded, executing, apply;
	static float maxScroll, scrollPos, expandPos;

	static std::thread* execThread;
	static AnNode* execNode;

	static bool hasPy, hasC, hasFt;
	static bool hasPy_s, hasC_s, hasFt_s;

	static void Insert(AnScript* scr, Vec2 pos = Vec2(100, 100));
	static void Insert(AnNode* node, Vec2 pos = Vec2(100, 100));
	static void Init(), Update(), Draw(), DrawSide(), DrawScene();
	static void Execute(), DoExecute(), DoExecute_Srv();
	static void Save(const string& s), SaveIn(), SaveOut();
	static void Load(const string& s), LoadIn(), LoadOut();
	static void OnExecLog(string s, bool e);

	static void OnAnimFrame();
};

#include "anbrowse.h"
#include "nodes/node_inputs.h"
#include "nodes/node_plot.h"
#include "nodes/node_volume.h"
#include "nodes/node_gromacs.h"
#include "nodes/node_recolor.h"
#include "nodes/node_addbond.h"