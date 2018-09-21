#pragma once
#include "../annode.h"

class Node_Recolor : public AnNode {
public:
	static const std::string sig;
	Node_Recolor();
    
	void Execute() override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};

class Node_Recolor_All : public Node_Recolor {
public:
	static const std::string sig;
	Node_Recolor_All();

	void Execute() override;
	void OnAnimFrame() override;

protected:
	std::vector<byte> data;
};