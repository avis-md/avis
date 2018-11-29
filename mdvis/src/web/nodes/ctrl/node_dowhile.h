#pragma once
#include "web/annode.h"
#include "ui/popups.h"

class Node_DoWhile : public AnNode {
public:
	enum class COMP : uint {
		GT,
		LT,
		NEQ,
		EQ
} type;

	static const std::string sig;
	Node_DoWhile();
	
	void DrawHeader(float& off) override;

	void Execute() override;

private:
	uint start = 0;

	std::vector<std::string> nodeNms;
	Popups::DropdownItem cDi, tDi;
	size_t _nodeCnt = ~0U;
};