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

	std::vector<std::pair<AnNode*, uint>> inputR, outputR;
	std::vector<union AnVarBase> inputVDef;
	std::vector<CVar> conV;

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
	virtual Vec2 DrawConn();
	virtual void Draw();
	virtual float GetHeaderSz();
	float hdrSz;
	virtual void DrawHeader(float& off);
	float midSz;
	virtual void DrawMiddle(float& off) {}
	float ftrSz;
	virtual void DrawFooter(float& off) {}
	float setSz;
	virtual void DrawSettings(float& off) {}
	virtual float DrawSide();
	float DrawLog(float off);
	virtual void DrawScene() {}
	void DrawToolbar();
	virtual void Execute() = 0;
	void ConnectTo(uint id, AnNode* tar, uint tarId); //out -> in
	void Save(std::ofstream& strm), Load(std::ifstream& strm);
	virtual void SaveIn(const std::string& path) {};
	virtual void LoadIn(const std::string& path) {};
	virtual void SaveOut(const std::string& path);
	virtual void LoadOut(const std::string& path);
	virtual void SaveConn(), ClearConn(), Reconn();

	static bool CanConn(std::string lhs, std::string rhs);

	virtual void CatchExp(char* c);

	virtual void OnSceneUpdate() {}
	virtual void OnAnimFrame() {}
	virtual void OnConn(bool o, int i) {}
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

	void CatchExp(char* c) override;
};

class CNode : public AnNode {
public:
	CNode(CScript*);

	std::vector<void*> inputV, outputV;

	void Execute() override;
	void Reconn() override;

	void CatchExp(char* c) override;
};

class FNode : public AnNode {
public:
	FNode(FScript*);

	std::vector<void*> inputV, outputV;

	void Execute() override;

	void CatchExp(char* c) override;
};