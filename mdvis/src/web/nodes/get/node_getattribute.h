#pragma once
#include "web/annode.h"
#include "ui/popups.h"

class Node_GetAttribute : public AnNode {
public:
	static const std::string sig;
	Node_GetAttribute();

    uint attrId;
    Popups::DropdownItem di;
    
	void Execute() override;
    void DrawHeader(float& off) override;
};