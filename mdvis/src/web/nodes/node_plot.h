#pragma once
#include "../annode.h"

class Node_Plot : public AnNode {
public:
	Node_Plot();

	Vec2 DrawConn() override;
	void Draw() override;
	float DrawSide() override;
	void Execute() override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
	std::vector<PyNode*> nodes;

protected:
	std::vector<float> valXs, valYs;
};