#pragma once
#include "../annode.h"

class Node_TraceTrj : public AnNode {
public:
	static const string sig;
	Node_TraceTrj();

	bool has = false, traceAll = true;

	std::vector<Vec3> path;
	int pathSz;
	Vec4 col = white();
    
	void Execute() override;
	void DrawScene() override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override;
};