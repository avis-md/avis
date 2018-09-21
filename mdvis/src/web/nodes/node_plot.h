#pragma once
#include "../annode.h"

class Node_Plot : public AnNode {
public:
	static const std::string sig;
	Node_Plot();

	void DrawHeader(float& y) override;
	void DrawFooter(float& y) override;
	void Execute() override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
	
	void OnConn(bool o, int i) override;

protected:
	bool useids;
	byte style;
	int xid, yid;

	std::vector<float> valXs, valYs;
};