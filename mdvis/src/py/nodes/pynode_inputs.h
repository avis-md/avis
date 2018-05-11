#pragma once
#include "../pynode.h"

class PyNode_Inputs : public PyNode {
public:
	PyNode_Inputs();

	Vec2 DrawConn() override;
	void Draw() override;
	float DrawSide() override { return 0; }
	void Execute() override;
	std::vector<PyNode*> nodes;
};

class PyNode_Inputs_ActPar : public PyNode_Inputs {
public:
	PyNode_Inputs_ActPar();
};