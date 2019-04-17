#pragma once
#include "web/annode_internal.h"

class Node_AddMesh : public AnNode {
public:
	INODE_DEF_H
	Node_AddMesh();
	~Node_AddMesh();

	void Update() override;
	void DrawHeader(float& off) override;
	void DrawScene(const RENDER_PASS pass) override;
	void RayTraceMesh(_Mesh& mesh) override;

	void Execute() override;
protected:
	bool dirty;
	int tsz;
	GLuint vao, vbos[2];
	static Shader shad;
	Vec4 col, _col;
	std::vector<float> poss, nrms;
};