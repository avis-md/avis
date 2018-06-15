#pragma once
#include "../annode.h"

class Node_Recolor : public AnNode {
public:
	Node_Recolor();
    
	void Execute() override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
};

class Node_Recolor_All : public Node_Recolor {
public:
	Node_Recolor_All();

	void Execute() override;
	void OnAnimFrame() override;

protected:
	std::vector<byte> data;
};