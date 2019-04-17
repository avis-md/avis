#pragma once
#include "web/annode_internal.h"
#include "ui/popups.h"
#include "utils/plot.h"

class Node_Remap : public AnNode {
public:
	INODE_DEF_H

	Node_Remap();
    
	std::vector<float> vals;
	float* val0;
	plt::remapdata graph;

	void Execute() override;
	void DrawMiddle(float& off) override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};