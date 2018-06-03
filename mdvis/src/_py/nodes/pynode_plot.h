#pragma once
#include "../pynode.h"

class PyNode_Plot : public PyNode {
public:
	PyNode_Plot();

	Vec2 DrawConn() override;
	void Draw() override;
	float DrawSide() override;
	void Execute() override;
	std::vector<PyNode*> nodes;

protected:
	std::vector<float> valXs, valYs;
};