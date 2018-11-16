#pragma once
#include "../annode.h"

class Node_ShowRange : public AnNode {
public:
	static const std::string sig;
	Node_ShowRange();
    
    bool invert;

    float rMin, rMax;
    
	void Execute() override;
	void OnAnimFrame() override { Execute(); }
    void DrawHeader(float& off) override;

	void Save(XmlNode* n) override;
	void Load(XmlNode* n) override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};