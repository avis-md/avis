#pragma once
#include "web/annode_internal.h"

class Node_SetCamera : public AnNode {
public:
	INODE_DEF_H
	Node_SetCamera();
    
	void Execute() override;

	void OnSceneUpdate() override;
	void OnAnimFrame() override;
};