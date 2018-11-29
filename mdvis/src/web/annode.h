#pragma once
#include "anscript.h"

const Vec3 NODE_COL_USR = Vec3(0.35f);
const Vec3 NODE_COL_NRM = Vec3(0.5f, 0.4f, 0.3f);
const Vec3 NODE_COL_IO = Vec3(0.3f, 0.3f, 0.5f);
const Vec3 NODE_COL_MOD = Vec3(0.3f, 0.5f, 0.3f);
const Vec3 NODE_COL_SPC = Vec3(0.5f, 0.3f, 0.3f);
const Vec3 NODE_COL_CTRL = Vec3(0.3f, 0.45f, 0.45f);

enum class AN_NODE_SCN : byte {
	NUM0 = 0x80,
	OCAM,
	NUM	
};
const std::string AN_NODE_SCNS[] = { "Camera (Set)" };

enum class AN_NODE_IN : byte {
	NUM0 = 0,
	PARS,
	PINFO,
	VEC,
	NUM
};
const std::string AN_NODE_INS[] = { "Particle data", "System info", "Vector" };

enum class AN_NODE_MOD {
	NUM0 = 0x20,
	GATTR,
	SATTR,
	RSCL,
	SBXC,
	NUM
};
const std::string AN_NODE_MODS[] = { "Get Param", "Set Param", "Set Radii Scale", "Set BBox Center" };

enum class AN_NODE_GEN {
	NUM0 = 0x40,
	BOND,
	TRJ,
	NUM
};
const std::string AN_NODE_GENS[] = { "Add Bonds", "Trace Trajectory" };

enum class AN_NODE_MISC {
	NUM0 = 0x60,
	PLOT,
	SRNG,
	ADJL,
	ADJLI,
	DOWHL,
	NUM
};
const std::string AN_NODE_MISCS[] = { "Plot graph", "Show Range", "To Adjacency List", "To Paired List", "Do While" };

enum class ANNODE_OP {
	NONE,
	LEFT,
	RIGHT,
	REMOVE
};

union AnVarBase {
	short s;
	int i;
	double d;
	uint64_t data;
};

typedef uint ANNODE_FLAGS;
#define AN_FLAG_NOSAVECONV 1
#define AN_FLAG_RUNONSEEK 2

class AnNode {
public:
	virtual ~AnNode() {}

	const bool saveConV = true;
	const bool runOnSeek = false;

	AnScript* script;
	bool selected;
	Vec2 pos;
	uint id;
	static float width;
	std::string title;
	Vec3 titleCol = NODE_COL_USR;
	static Vec4 bgCol;

	bool expanded = true;
	bool showDesc = false, showSett = false;
	bool canTile = false;
	bool executing = false;
	ANNODE_OP op;

	bool logExpanded = true;
	byte logMask = 7;
	int logOffset = 0;
	std::vector<std::pair<byte, std::string>> log;

	struct nodecon {
		AnNode* first;
		uint second;
		bool use;
		nodecon(AnNode* f = 0, uint s = 0, bool use = true) : first(f), second(s), use(true) {}
		CVar& getconv() { return first->conV[second]; }
		void*& getval() { return getconv().value; }
	};
	std::vector<nodecon> inputR;
	std::vector<std::vector<nodecon>> outputR;
	std::vector<union AnVarBase> inputVDef;
	std::vector<CVar> conV;

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
	
	static void Init();

	bool Select();
	virtual float GetHeaderSz();
	virtual Vec2 DrawConn();
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

	virtual void AddInput(), AddOutput();

	virtual void Execute() = 0;
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
	virtual void OnValChange(int i) {}
protected:
	AnNode(AnScript* scr, ANNODE_FLAGS flags = 0);

	static Texture tex_circle_open, tex_circle_conn;
};

class PyNode : public AnNode {
public:
	PyNode(PyScript*);
	
	std::vector<PyVar> inputV, outputV;
	std::vector<VarVal> outputVC;
	
	void Execute() override;
	void WriteFrame(int f) override;
	void RemoveFrames() override;

	void CatchExp(char* c) override;
};

class CNode : public AnNode {
public:
	CNode(CScript*);

	std::vector<void*> inputV, outputV;

	void Execute() override;
	void WriteFrame(int f) override;
	void RemoveFrames() override;

	void Reconn() override;

	void CatchExp(char* c) override;
};

class FNode : public AnNode {
public:
	FNode(FScript*);

	std::vector<void*> inputV, outputV;
	
	void Execute() override;
	void WriteFrame(int f) override;
	void RemoveFrames() override;

	void CatchExp(char* c) override;
};