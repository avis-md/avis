#pragma once
#include "../pynode.h"


class PyNode_Plot : public PyNode {
public:
	PyNode_Plot();

	void Draw() override;
	void DrawConn() override;
	void Execute() override;
	std::vector<PyNode*> nodes;

protected:
	std::vector<float> valXs, valYs;
};