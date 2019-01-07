#pragma once
#include "web/annode_internal.h"

class Node_Inputs : public AnNode {
public:
	INODE_DEF_H
	Node_Inputs();

	enum class FILTER : int {
		VIS = 1,
		CLP = 2
	};
	uint filter;

	static uint parcount;

	void DrawHeader(float& off) override;

	void Execute() override;

	void SaveIn(const std::string& path) override;
	void LoadIn(const std::string& path) override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override {}

protected:
	std::vector<glm::dvec3> vpos, vvel;
	std::vector<short> vtyp;

	double* poss, *vels;
	short* typs;
};