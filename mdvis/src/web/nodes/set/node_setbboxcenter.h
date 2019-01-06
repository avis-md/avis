#pragma once
#include "web/annode_internal.h"

class Node_SetBBoxCenter : public AnNode {
public:
	INODE_DEF_H

	Node_SetBBoxCenter();
    
	void Execute() override;
};