#pragma once
#include "../annode.h"

class Node_Inputs : public AnNode {
public:
	static const std::string sig;
	Node_Inputs();

	float DrawSide() override { return 0; }
	void Execute() override;

	void SaveIn(const std::string& path) override;
	void LoadIn(const std::string& path) override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override {}
};

class Node_Inputs_ActPar : public Node_Inputs {
public:
	static const std::string sig;
	Node_Inputs_ActPar();
	void Execute() override;
};

class Node_Inputs_SelPar : public Node_Inputs {
public:
	static const std::string sig;
	enum class SELTYPE {
		RSL, RES, PAR
	} type;
	std::string tv_resNm;
	uint tv_resId;
	uint tv_atomId;

	Node_Inputs_SelPar();
	void DrawHeader(float& off) override;
	void Execute() override;
};