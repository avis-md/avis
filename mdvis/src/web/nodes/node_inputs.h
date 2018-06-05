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
};

class Node_Inputs_ActPar : public Node_Inputs {
public:
	Node_Inputs_ActPar();
	void Execute() override;
};