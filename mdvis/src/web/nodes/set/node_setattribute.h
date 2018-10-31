#pragma once
#include "web/annode.h"
#include "ui/popups.h"

class Node_SetAttribute : public AnNode {
public:
	static const std::string sig;
	Node_SetAttribute();

    uint attrId;
    Popups::DropdownItem di;
    
	void Execute() override;
    void DrawHeader(float& off) override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};