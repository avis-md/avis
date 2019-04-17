#pragma once
#include "web/annode_internal.h"

class Node_SetRadScl : public AnNode {
public:
	INODE_DEF_H

	Node_SetRadScl();

	bool canReset = false;
    
	void Execute() override;
	void DrawFooter(float& off) override;
	void OnAnimFrame() override { Execute(); }
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};