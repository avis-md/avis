#pragma once
#include "Engine.h"
#include "anscript.h"
#include "annode.h"
#include "anbrowse.h"
#include "arrayview.h"

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
	
	static std::string nodesPath;

	static AnNode* selConnNode;
	static uint selConnId;
	static bool selConnIdIsOut, selPreClear;
	static Vec2 selConnPos;
	static Vec4 selConnCol;
	static AnScript* selScript;
	static uint selSpNode;

	static std::string activeFile;
	static std::vector<pAnNode> nodes;
	static std::mutex execNLock;
	static int execN;
	static bool abortExec;

	static bool drawFull, expanded, executing, apply;
	static float maxScroll, scrollPos, expandPos;
	static int execFrame, realExecFrame, execdFrame;
	static float drawLerp;
	static bool invertRun, runOnFrame;
	static bool highContrast;
	
	static float zoomOut;
	static float zoomRatio;

	static std::thread* execThread;
	static AnNode* execNode;
	static uint currNode, nextNode;

	static bool hasPy, hasC, hasFt;
	static bool hasPy_s, hasC_s, hasFt_s;

	static bool waitBrowse;

	static void Clear(), Clear0();
	static void Insert(const pAnNode& node, Vec2 pos = Vec2(100, 100));
	static void Init(), Update(), Draw(), DrawSide(), DrawScene(const RENDER_PASS), DrawOverlay();
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

	static std::string ConvertName(const std::string& name);
};

#include "annode_internal.h"