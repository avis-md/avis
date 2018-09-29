#pragma once
#include "../annode.h"

class Node_Inputs : public AnNode {
public:
	static const std::string sig;
	Node_Inputs();

	static uint frame;

	float DrawSide() override { return 0; }
	void Execute() override;

	void SaveIn(const std::string& path) override;
	void LoadIn(const std::string& path) override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override {}
};