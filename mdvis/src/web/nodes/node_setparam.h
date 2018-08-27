#pragma once
#include "../annode.h"
#include "ui/popups.h"

class Node_SetParam : public AnNode {
public:
	static const string sig;
	Node_SetParam();

    uint paramId;
    Popups::DropdownItem di;
    
	void Execute() override;
    void DrawHeader(float& off) override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
};