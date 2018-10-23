#pragma once
#include "web/annode.h"

class Node_Vector : public AnNode {
public:
	static const std::string sig;
	Node_Vector();
    
	void Execute() override;

private:
	glm::dvec3 vec;
};