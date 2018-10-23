#pragma once
#include "Engine.h"
#include "anscript.h"
#include "annode.h"
#include "anbrowse.h"
#include "creader.h"
#include "pyreader.h"
#include "freader.h"

#define EXT_PS ".py"
constexpr int EXT_PS_SZ = strlen_c(EXT_PS);
#define EXT_CS ".cpp"
constexpr int EXT_CS_SZ = strlen_c(EXT_CS);
#define EXT_FS ".f90"
constexpr int EXT_FS_SZ = strlen_c(EXT_FS);
#define EXT_ANSV ".anl"
constexpr int EXT_ANSV_SZ = strlen_c(EXT_ANSV);

class AnWeb {
public:
	static bool lazyLoad;
	
	static AnNode* selConnNode;
	static uint selConnId;
	static bool selConnIdIsOut, selPreClear;
	static AnScript* selScript;
	static byte selSpNode;

	static std::string activeFile;
	static std::vector<AnNode*> nodes;

	static bool drawFull, expanded, executing, apply;
	static float maxScroll, scrollPos, expandPos;
	static int execFrame;
	static float drawLerp;

	static std::thread* execThread;
	static AnNode* execNode;

	static bool hasPy, hasC, hasFt;
	static bool hasPy_s, hasC_s, hasFt_s;

	static void Clear(), Clear0();
	static void Insert(AnScript* scr, Vec2 pos = Vec2(100, 100));
	static void Insert(AnNode* node, Vec2 pos = Vec2(100, 100));
	static void Init(), Update(), Draw(), DrawSide(), DrawScene();
	static void Execute(bool all), DoExecute(bool all), _DoExecute(), DoExecute_Srv();
	static void ApplyFrameCount(int f), WriteFrame(uint f), ReadFrame(uint f), RemoveFrames();
	static void Save(const std::string& s), SaveIn(), SaveOut();
	static void Load(const std::string& s), LoadIn(), LoadOut();
	static void CheckChanges();
	static void SaveConn(), ClearConn(), Reconn();
	static void OnExecLog(std::string s, bool e);

	static void Serialize(XmlNode* nd);
	static void Deserialize(XmlNode* nd);

	static void OnSceneUpdate();
	static void OnAnimFrame();
};

#include "anbrowse.h"
#include "nodes/in/node_vector.h"
#include "nodes/node_inputs.h"
#include "nodes/node_info.h"
#include "nodes/node_addbond.h"
#include "nodes/node_camera.h"
#include "nodes/node_plot.h"
#include "nodes/node_setparam.h"
#include "nodes/node_showrange.h"
#include "nodes/node_setradscl.h"
#include "nodes/set/node_setbboxcenter.h"
#include "nodes/node_tracetrj.h"
#include "nodes/node_volume.h"
#include "nodes/node_remap.h"
#include "nodes/node_gromacs.h"
#include "nodes/misc/node_adjlist.h"