#pragma once
#include "web/annode_internal.h"

class Node_Downsample : public AnNode {
public:
	INODE_DEF_H
	Node_Downsample();

	void Execute() override;

	void OnConn(bool o, int i) override;

protected:
    std::vector<double> data;
    std::vector<int> dims;
};