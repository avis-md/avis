#pragma once
#include "web/annode_internal.h"

class Node_AddSurface : public AnNode {
public:
	INODE_DEF_H
	Node_AddSurface();
	~Node_AddSurface();
	
	void Execute() override;

	void Update() override;
	void DrawScene(const RENDER_PASS pass) override;
protected:
	static Shader marchProg, drawProg;

	std::vector<float> data;
	int shape[3];
	float cutoff;

	static size_t bufSz, outSz;
	static uint genSz;
	static GLuint inBuf, inBufT, query;
	GLuint vao, outPos, outNrm;

	static std::mutex lock;

	static void Init();
	void Set();
	void ExecMC();

	static const float triTable[256*15];
	static GLuint triBuf, triBufT;
};