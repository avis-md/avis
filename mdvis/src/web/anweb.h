#pragma once
#include "Engine.h"
#include "anscript.h"
#include "annode.h"
#include "anbrowse.h"
#include "pyreader.h"
#include "creader.h"

enum class AN_NODE_SCN : byte {
	NUM0 = 0x80,
	OCAM,
	NUM	
};
const string AN_NODE_SCNS[] = { "Camera (Set)" };

enum class AN_NODE_IN : byte {
	NUM0 = 0,
	SELPAR,
	NUM
};
const string AN_NODE_INS[] = { "Sel'd Particles" };

enum class AN_NODE_MOD {
	NUM0 = 0x20,
	RECOL,
	RECOLA,
	PARAM,
	NUM
};
const string AN_NODE_MODS[] = { "Recolor", "Recolor All", "Set Param" };

enum class AN_NODE_GEN {
	NUM0 = 0x40,
	BOND,
	VOL,
	NUM
};
const string AN_NODE_GENS[] = { "Add Bonds", "Add Volume" };

enum class AN_NODE_MISC {
	NUM0 = 0x60,
	PLOT,
	NUM
};
const string AN_NODE_MISCS[] = { "Plot graph" };

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
	static void CheckChanges();
	static void SaveConn(), ClearConn(), Reconn();
	static void OnExecLog(string s, bool e);

	static void OnSceneUpdate();
	static void OnAnimFrame();
};

#include "anbrowse.h"
#include "nodes/node_addbond.h"
#include "nodes/node_camera.h"
#include "nodes/node_gromacs.h"
#include "nodes/node_inputs.h"
#include "nodes/node_plot.h"
#include "nodes/node_recolor.h"
#include "nodes/node_setparam.h"
#include "nodes/node_volume.h"