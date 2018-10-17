#pragma once
#include "web/annode.h"

class Node_AdjList : public AnNode {
public:
	static const std::string sig;
	Node_AdjList();

	void Execute() override;

protected:
	std::vector<int> conns;
	int count, listsize;
};

class Node_AdjListI : public AnNode {
public:
	static const std::string sig;
	Node_AdjListI();

	void Execute() override;

protected:
	std::vector<int> conns;
	int count;
};