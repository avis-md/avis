#pragma once
#include "../annode.h"

class Node_SetRadScl : public AnNode {
public:
	static const std::string sig;
	Node_SetRadScl();
    
	void Execute() override;
	void OnAnimFrame() override { Execute(); }
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};