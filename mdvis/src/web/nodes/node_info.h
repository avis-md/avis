#pragma once
#include "../annode.h"

class Node_Info : public AnNode {
public:
	static const std::string sig;
	Node_Info();
	
	void Execute() override;

	void SaveIn(const std::string& path) override {}
	void LoadIn(const std::string& path) override {}
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override {}
};