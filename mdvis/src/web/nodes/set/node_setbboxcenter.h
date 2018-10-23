#pragma once
#include "web/annode.h"

class Node_SetBBoxCenter : public AnNode {
public:
	static const std::string sig;
	Node_SetBBoxCenter();
    
	void Execute() override;
};