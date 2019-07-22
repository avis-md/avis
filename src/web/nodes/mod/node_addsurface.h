#pragma once
#include "web/annode_internal.h"

class Node_AddSurface : public AnNode {
public:
	INODE_DEF_H
	Node_AddSurface();
	~Node_AddSurface();

	void Update() override;
	void DrawHeader(float& off) override;
	void DrawScene(const RENDER_PASS pass) override;

	void RayTraceMesh(_Mesh& mesh) override;
	
	void Execute() override;
protected:
	static Shader marchProg, drawProg;

	std::vector<float> data;
	int shape[3];
	float cutoff;
	bool invert = false;

	std::vector<Vec3> resultPos, resultNrm;

	static size_t bufSz, outSz;
	static int maxBufSz;
	static uint genSz;
	static GLuint inBuf, inBufT, query;
	GLuint vao, outPos, outNrm, tmpPos, tmpNrm;

	static std::mutex lock;

	static void Init();
	void InitBuffers();

	void ResizeInBuf(int), ResizeOutBuf(int), ResizeTmpBuf(int);
	void SetInBuf(void*, int);
	int ExecMC(glm::ivec3 offset, glm::ivec3 size, glm::ivec3 rsize);

	static const int triTable[256*15];
	static GLuint triBuf, triBufT;
};