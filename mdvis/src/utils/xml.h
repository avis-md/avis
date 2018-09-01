#pragma once
#include "Engine.h"

struct XmlNode {
	std::string name = "", value = "";
	std::unordered_map<string, string> params;
	std::vector<XmlNode> children;
};

class Xml {
public:
	static XmlNode* Parse(const string& path);
protected:
	static bool Read(string& s, uint& pos, XmlNode* parent);
};