#pragma once
#include "web/annode.h"

class Node_AddSurface : public AnNode {
public:
	static const std::string sig;
	Node_AddSurface();
	~Node_AddSurface();
	
	void Execute() override;
	void Update() override;
	void DrawScene() override;
	//void DrawSettings(float& off) override;
	//float DrawSide() override;
	//void SaveOut(const std::string& path) override {}
	//void LoadOut(const std::string& path) override;
protected:
	static bool initd;
	PROGDEF_H(marcherProg, 10);
	PROGDEF_H(drawProg, 10);

	std::vector<byte> data;

	static size_t bufSz, outSz;
	static uint genSz;
	static GLuint inBuf, vao, outPos, outNrm, query;

	static std::mutex lock;

	static void Init();
	void Set();
	static void ExecMC();
};