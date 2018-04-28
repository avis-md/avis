#pragma once
#include "../pynode.h"

class PyNode_Inputs : public PyNode {
public:
	PyNode_Inputs();

	void Draw() override;
	void DrawConn() override {};
	void Execute() override;
	std::vector<PyNode*> nodes;

protected:
	std::vector<float> valXs, valYs;
};