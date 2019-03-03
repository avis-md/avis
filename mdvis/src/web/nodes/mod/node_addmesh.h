#pragma once
#include "web/annode_internal.h"

class Node_AddMesh : public AnNode {
public:
	INODE_DEF_H
	Node_AddMesh();
	~Node_AddMesh();
	
	void Execute() override;

	void Update() override;
	void DrawHeader(float& off) override;
	void DrawScene() override;
protected:
	bool dirty;
	int tsz;
	GLuint vao, vbos[2];
	static Shader shad;
	Vec4 col;
	std::vector<float> poss, nrms;
};