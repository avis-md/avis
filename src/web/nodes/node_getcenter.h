#pragma once
#include "../annode.h"

class Node_GetCenter : public AnNode {
public:
	static const std::string sig;
	Node_GetCenter();

	void Execute() override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;

protected:
	bool periodic;
};