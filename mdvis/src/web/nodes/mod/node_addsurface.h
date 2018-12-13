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
protected:
	static bool initd;
	PROGDEF_H(marcherProg, 10);
	PROGDEF_H(drawProg, 10);

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