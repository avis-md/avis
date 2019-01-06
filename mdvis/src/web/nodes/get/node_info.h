#pragma once
#include "web/annode_internal.h"

class Node_Info : public AnNode {
public:
	INODE_DEF_H
	Node_Info();
	
	void Execute() override;

	void SaveIn(const std::string& path) override {}
	void LoadIn(const std::string& path) override {}
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override {}
};