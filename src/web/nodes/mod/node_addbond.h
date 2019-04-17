#pragma once
#include "web/annode_internal.h"

class Node_AddBond : public AnNode {
public:
	INODE_DEF_H
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