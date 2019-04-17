#pragma once
#include "web/annode_internal.h"
#include "ui/popups.h"

class Node_SetAttribute : public AnNode {
public:
	INODE_DEF_H

	Node_SetAttribute();

	uint attrId, _attrId, attrSz;
	std::vector<std::string> attrs;
	Popups::DropdownItem di;
	bool timed;

	void Execute() override;
	void DrawHeader(float& off) override;
	
	void Save(XmlNode* n) override;
	void Load(XmlNode* n) override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};