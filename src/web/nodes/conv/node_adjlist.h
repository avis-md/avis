#pragma once
#include "web/annode_internal.h"

class Node_AdjList : public AnNode {
public:
	INODE_DEF_H
	Node_AdjList();

	void Execute() override;

protected:
	std::vector<int> conns;
	int count, listsize;
};

class Node_AdjListI : public AnNode {
public:
	INODE_DEF_H
	Node_AdjListI();

	void Execute() override;

protected:
	std::vector<int> conns;
	int count;
};