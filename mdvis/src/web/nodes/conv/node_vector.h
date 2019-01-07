#pragma once
#include "web/annode_internal.h"

class Node_ToVector : public AnNode {
public:
	INODE_DEF_H
	Node_ToVector();
    
	void Execute() override;
private:
	glm::dvec3 vec;
};