#pragma once
#include "anweb.h"
#include "anscript.h"

const Vec3 NODE_COL_USR = Vec3(0.35f);
const Vec3 NODE_COL_NRM = Vec3(0.5f, 0.4f, 0.3f);
const Vec3 NODE_COL_IO = Vec3(0.3f, 0.3f, 0.5f);
const Vec3 NODE_COL_MOD = Vec3(0.3f, 0.5f, 0.3f);
const Vec3 NODE_COL_SPC = Vec3(0.5f, 0.3f, 0.3f);
const Vec3 NODE_COL_CTRL = Vec3(0.3f, 0.45f, 0.45f);

struct AnCol {
	static Vec4 conn_scalar;
	static Vec4 conn_vector;
};

enum class ANNODE_OP {
	NONE,
	LEFT,
	RIGHT,
	REMOVE
};

typedef uint ANNODE_FLAGS;
#define AN_FLAG_NOSAVECONV 1
#define AN_FLAG_RUNONSEEK 2
#define AN_FLAG_RUNONVALCHG 4

class AnNode {
public:
	virtual ~AnNode() {}
	
	static void Init();

	const ANNODE_FLAGS flags;

	uint id;
	pAnScript_I script;
	std::string title;
	Vec3 titleCol = NODE_COL_USR;
	bool selected;
	Vec2 pos;
	static float width;
	float height;

	bool canexec;
	bool execd;
	std::vector<AnNode*> parents;
	std::mutex execMutex;

	bool expanded = true;
	bool canTile = false;
	bool executing = false;
	bool showDesc = false, showSett = false;
	ANNODE_OP op;

	bool logExpanded = true;
	byte logMask = 7;
	int logOffset = 0;
	std::vector<std::pair<byte, std::string>> log;
	
	struct nodecon {
		AnNode* first;
		uint second;
		bool use;
		bool hoverdel;
		bool execd;
		nodecon(AnNode* f = 0, uint i = 0, bool use = true)
			: first(f), second(i), use(true), hoverdel(false) {}
		CVar& getconv() { return first->conV[second]; }
		void* getval();
		AnScript::Var& getvar();
		int* getdim(int i); //shorthand
	};
	std::vector<nodecon> inputR;
	std::vector<std::vector<nodecon>> outputR;
	std::vector<CVar> conV;
	short&  getval_s(const uint i);
	int&    getval_i(const uint i);
	double& getval_d(const uint i);
	
	std::vector<std::vector<VarVal>> conVAll;

	struct ConnInfo {
		bool cond;
		std::string mynm;
		std::string mytp;
		AnNode* tar;
		std::string tarnm;
		std::string tartp;
	};
	std::vector<ConnInfo> _connInfo;

	void ResizeIO(AnScript* scr);

	bool Select();
	virtual float GetHeaderSz();

	virtual void Update() {}
	void DrawBack();
	Vec2 DrawConn();
	static void DrawMouseConn();
	virtual void Draw();
	virtual float DrawSide();
	void DrawDefVal(int i, float y);
	float hdrSz;
	virtual void DrawHeader(float& off);
	float midSz;
	virtual void DrawMiddle(float& off) {}
	float ftrSz;
	virtual void DrawFooter(float& off) {}
	float setSz;
	virtual void DrawSettings(float& off) {}
	float DrawLog(float off);
	virtual void DrawScene() {}
	void DrawToolbar();

	virtual void AddInput();
	virtual void AddOutput(const CVar& cv = CVar());

	virtual void PreExecute();
	bool TryExecute();
	virtual void Execute() = 0;
	void ExecuteNext();
	void ApplyFrameCount(int f);
	virtual void WriteFrame(int f);
	bool ReadFrame(int f);
	virtual void RemoveFrames();

	virtual void Save(XmlNode* n) {}
	virtual void Load(XmlNode* n) {}
	virtual void SaveIn(const std::string& path) {};
	virtual void LoadIn(const std::string& path) {};
	virtual void SaveOut(const std::string& path);
	virtual void LoadOut(const std::string& path);
	virtual void SaveConn(), ClearConn(), Reconn();

	bool HasConnI(int i), HasConnO(int i);

	static bool CanConn(std::string lhs, std::string rhs);
	void ConnectTo(uint id, AnNode* tar, uint tarId); //out -> in
	void Disconnect(uint id, bool out);

	virtual void CatchExp(char* c);

	virtual void OnSceneUpdate() {}
	virtual void OnAnimFrame();
	virtual void OnConn(bool o, int i) {}
	virtual void OnValChange(int i);
protected:
	AnNode(pAnScript_I scr); //for user scripts
	AnNode(pAnScript_I scr, ANNODE_FLAGS flags); //for internal scripts

	static void _Execute(AnNode* n);

	void IAddConV(void*);
	void IAddConV(void*, std::initializer_list<int*>, std::initializer_list<int>);
};
typedef std::shared_ptr<AnNode> pAnNode;

#include "cc/cnode.h"
#include "py/pynode.h"
#include "ft/fnode.h"