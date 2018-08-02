#pragma once
#include "../annode.h"

class Node_Camera_Out : public AnNode {
public:
	Node_Camera_Out();
    
	void Execute() override;

	void OnSceneUpdate() override;
};