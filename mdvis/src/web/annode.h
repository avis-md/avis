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
};

class AnNode {
public:
	virtual ~AnNode() {}
	
	AnScript* script;
	bool selected;
	Vec2 pos;
	uint id;
	static float width;
	string title;
	Vec3 titleCol = Vec3(0.35f, 0.35f, 0.35f);

	bool expanded = true;
	bool showDesc = false, showSett = false;
	bool canTile = false;
	bool executing = false;
	ANNODE_OP op;
	int settSz = 0;
	int hdSz = 0;

	bool logExpanded = true;
	byte logMask = 7;
	int logOffset = 0;
	std::vector<std::pair<byte, string>> log;

	std::vector<std::pair<AnNode*, uint>> inputR, outputR;
	std::vector<union AnVarBase> inputVDef;
	std::vector<CVar> conV;

	struct ConnInfo {
		bool cond;
		string mynm;
		string mytp;
		AnNode* tar;
		string tarnm;
		string tartp;
	};
	std::vector<ConnInfo> _connInfo;
	
	static void Init();

	bool Select();
	virtual Vec2 DrawConn();
	virtual void Draw();
	virtual float GetHeaderSz();
	virtual void DrawHeader(float& off);
	virtual void DrawSettings(float& off) {}
	virtual float DrawSide();
	float DrawLog(float off);
	virtual void DrawScene() {}
	void DrawToolbar();
	virtual void Execute() = 0;
	void ConnectTo(uint id, AnNode* tar, uint tarId); //out -> in
	void Save(std::ofstream& strm), Load(std::ifstream& strm);
	virtual void SaveIn(const string& path) {};
	virtual void LoadIn(const string& path) {};
	virtual void SaveOut(const string& path);
	virtual void LoadOut(const string& path);
	virtual void SaveConn(), ClearConn(), Reconn();

	virtual void OnSceneUpdate() {}
	virtual void OnAnimFrame() {}
protected:
	AnNode(AnScript* scr);

	static Texture* tex_circle_open, *tex_circle_conn;
};

class PyNode : public AnNode {
public:
	PyNode(PyScript*);
	
	std::vector<PyVar> inputV, outputV;
	
	void Execute() override;
};

class CNode : public AnNode {
public:
	CNode(CScript*);

	std::vector<void*> inputV, outputV;

	void Execute() override;
	void Reconn() override;
};