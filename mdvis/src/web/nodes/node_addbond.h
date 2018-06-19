#pragma once
#include "../annode.h"

class Node_AddBond : public AnNode {
public:
	Node_AddBond();
    
	void Execute() override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
	
protected:
	byte animId;
};