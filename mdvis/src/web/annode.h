#pragma once
#include "anscript.h"

enum class ANNODE_OP {
	NONE,
	LEFT,
	RIGHT,
	REMOVE
};

union AnVarBase {
	int i;
	float f;
	double d;
	uint64_t data;
};

class AnNode {
public:
	virtual ~AnNode() {}
	
	AnScript* script;
	bool selected;
	Vec2 pos;
	uint id;
	static float width;
	std::string title;
	Vec3 titleCol = Vec3(0.35f);
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

	virtual void Execute() = 0;
	void ApplyFrameCount(int f);
	virtual void WriteFrame(int f) {};
	bool ReadFrame(int f);
	virtual void RemoveFrames();

	void Save(std::ofstream& strm), Load(std::ifstream& strm);
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
	virtual void OnAnimFrame() {}
	virtual void OnConn(bool o, int i) {}
	virtual void OnValChange(int i) {}
protected:
	AnNode(AnScript* scr);

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