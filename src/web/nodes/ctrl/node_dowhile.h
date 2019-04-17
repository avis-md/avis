#pragma once
#include "web/annode_internal.h"
#include "ui/popups.h"

class Node_DoWhile : public AnNode {
public:
	INODE_DEF_H
	Node_DoWhile();

	enum class COMP : uint {
		GT,
		LT,
		NEQ,
		EQ
	} type;
	
	void DrawHeader(float& off) override;

	void Execute() override;

private:
	uint start = 0;

	std::vector<std::string> nodeNms;
	Popups::DropdownItem cDi, tDi;
	size_t _nodeCnt = ~0U;
};