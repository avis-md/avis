#pragma once
#include "../annode.h"

class Node_Inputs : public AnNode {
public:
	Node_Inputs();

	Vec2 DrawConn() override;
	void Draw() override;
	float DrawSide() override { return 0; }
	void Execute() override;
	std::vector<PyNode*> nodes;

	void SaveIn(const string& path) override;
	void LoadIn(const string& path) override;
};

class Node_Inputs_ActPar : public Node_Inputs {
public:
	Node_Inputs_ActPar();
	void Execute() override;
};