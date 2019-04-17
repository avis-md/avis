#pragma once
#include "web/annode_internal.h"

class Node_ShowRange : public AnNode {
public:
	INODE_DEF_H

	Node_ShowRange();
    
    bool invert = false;
	bool canReset = false;

    float rMin, rMax;
    
	void Execute() override;
	void OnAnimFrame() override { Execute(); }
    void DrawHeader(float& off) override;
	void DrawFooter(float& off) override;

	void Save(XmlNode* n) override;
	void Load(XmlNode* n) override;
	void SaveOut(const std::string& path) override {}
	void LoadOut(const std::string& path) override;
};