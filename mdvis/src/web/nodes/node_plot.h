#pragma once
#include "../annode.h"

class Node_Plot : public AnNode {
public:
	static const std::string sig;
	Node_Plot();

	void DrawFooter(float& y) override;
	void Execute() override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
	
	void OnConn(bool o, int i) override;
	void OnValChange(int i) override;

protected:
	bool useids;
	byte style;

	std::vector<float> valXs;
	std::vector<std::vector<float>> valYs;
	std::vector<float*> _valYs;
};