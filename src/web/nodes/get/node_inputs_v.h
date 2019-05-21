#pragma once
#include "web/annode_internal.h"

class Node_InputsV : public AnNode {
public:
	INODE_DEF_H
	Node_InputsV();

	void Execute() override;

	void SaveIn(const std::string& path) override {}
	void LoadIn(const std::string& path) override {}
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override {}

protected:
	int szs[3];
	double* data;
};