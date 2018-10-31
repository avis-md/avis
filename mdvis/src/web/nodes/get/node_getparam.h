#pragma once
#include "web/annode.h"
#include "ui/popups.h"

class Node_GetParam : public AnNode {
public:
	static const std::string sig;
	Node_GetParam();

    uint paramId;
    Popups::DropdownItem di;
    
	void Execute() override;
    void DrawHeader(float& off) override;
};