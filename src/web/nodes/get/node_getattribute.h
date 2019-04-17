#pragma once
#include "web/annode_internal.h"
#include "ui/popups.h"

class Node_GetAttribute : public AnNode {
public:
	INODE_DEF_H
	Node_GetAttribute();

    uint attrId;
    Popups::DropdownItem di;
    
	void Execute() override;
    void DrawHeader(float& off) override;
	
	void Save(XmlNode* n) override;
	void Load(XmlNode* n) override;
};