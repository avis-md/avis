#pragma once
#include "web/annode_internal.h"

class Node_Subgraph_In : public AnNode {
public:
	INODE_DEF_NOREG_H
	Node_Subgraph_In();

	void Execute() override;
	void OnConn(bool o, int i) override;
};