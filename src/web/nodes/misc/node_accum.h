#pragma once
#include "web/annode_internal.h"

class Node_Accum : public AnNode {
public:
	INODE_DEF_H
	Node_Accum();

	void DrawHeader(float& off) override;
	void Execute() override;
	void OnConn(bool o, int i) override;
protected:
	std::vector<char> vals;
	bool block;
};