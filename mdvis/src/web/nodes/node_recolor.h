#pragma once
#include "../annode.h"

class Node_Recolor : public AnNode {
public:
	Node_Recolor();
    
	void Execute() override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
};