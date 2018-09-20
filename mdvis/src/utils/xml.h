#pragma once
#include "Engine.h"

struct XmlNode {
	string name = "", value = "";
	std::unordered_map<string, string> params;
	std::vector<XmlNode> children;

	XmlNode(string nm = "", string vl = "") : name(nm), value(vl) {}

	XmlNode* addchild(string nm = "", string vl = "");
};

class Xml {
public:
	static XmlNode* Parse(const string& path);
	static XmlNode FromVec(string nm, Vec2 v);
	static XmlNode FromVec(string nm, Vec3 v);
	static XmlNode FromVec(string nm, Vec4 v);

	static void Write(XmlNode* node, const string& path);
protected:
	static bool Read(string& s, uint& pos, XmlNode* parent);

	static void DoWrite(XmlNode* n, std::ofstream& strm, int ind);
};