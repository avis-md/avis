#pragma once
#include "node_inputs.h"

class Node_Gromacs : public Node_Inputs {
public:
	Node_Gromacs();

	std::string file;

	Vec2 DrawConn() override {
		return Vec2(width, 36 + 17 * 4);
	}
	void Draw() override;
	void Execute() override;

	void SaveIn(const std::string& path) override {}
	void LoadIn(const std::string& path) override {}
};