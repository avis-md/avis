#pragma once
#include "../annode.h"

class Node_Plot : public AnNode {
public:
	static const string sig;
	Node_Plot();

	void Draw() override;
	float DrawSide() override;
	void Execute() override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
	
protected:
	std::vector<float> valXs, valYs;
};