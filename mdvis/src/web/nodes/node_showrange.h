#pragma once
#include "../annode.h"

class Node_ShowRange : public AnNode {
public:
	static const string sig;
	Node_ShowRange();
    
    bool invert;

    float rMin = 0, rMax = 1;
    
	void Execute() override;
    void DrawHeader(float& off) override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
};