#pragma once
#include "../annode.h"
#include "ui/popups.h"
#include "utils/plot.h"

class Node_Remap : public AnNode {
public:
	static const string sig;
	Node_Remap();
    
	std::vector<float> vals;
	float* val0;
	plt::remapdata graph;

	void Execute() override;
	void DrawMiddle(float& off) override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
};