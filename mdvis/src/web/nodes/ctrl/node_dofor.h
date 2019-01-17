#pragma once
#include "web/annode_internal.h"

class Node_DoFor : public AnNode {
public:
	INODE_DEF_H
	Node_DoFor();

	void Execute() override;

	static int _i, _im, _sz;
	static uint _st;
};

class Node_DoForEnd : public AnNode {
public:
	INODE_DEF_H
	Node_DoForEnd();

	void Execute() override;
};