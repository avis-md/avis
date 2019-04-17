#pragma once
#include "web/annode_internal.h"

class Node_TraceTrj : public AnNode {
public:
	INODE_DEF_H
	Node_TraceTrj();

	bool has = false, traceAll = true;

	std::vector<Vec3> path;
	int pathSz;
	Vec4 col = white();
    
	void Execute() override;
	void DrawScene() override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};