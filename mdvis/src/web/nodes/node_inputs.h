#pragma once
#include "../annode.h"

class Node_Inputs : public AnNode {
public:
	Node_Inputs();

	Vec2 DrawConn() override {
		return Vec2(width, 19 + 17 * 4);
	}
	void Draw() override;
	float DrawSide() override { return 0; }
	void Execute() override;

	void SaveIn(const string& path) override;
	void LoadIn(const string& path) override;
	void SaveOut(const string& path) override {}
	void LoadOut(const string& path) override {}
};

class Node_Inputs_ActPar : public Node_Inputs {
public:
	Node_Inputs_ActPar();
	void Execute() override;
};

class Node_Inputs_SelPar : public Node_Inputs {
public:
	enum class SELTYPE {
		RSL, RES, PAR
	} type;

	Node_Inputs_SelPar();
	void DrawHeader(float& off) override;
	void Execute() override;
};