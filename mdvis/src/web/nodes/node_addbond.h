#pragma once
#include "../annode.h"

class Node_AddBond : public AnNode {
public:
	static const std::string sig;
	Node_AddBond();
	~Node_AddBond();
	
	void Execute() override;
	void DrawSettings(float& off) override;
	float DrawSide() override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
	
protected:
	byte animId;
};