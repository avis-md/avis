#pragma once
#include "Engine.h"

struct XmlNode {
	std::string name = "", value = "";
	std::unordered_map<std::string, std::string> params;
	std::vector<XmlNode> children;

	XmlNode(std::string nm = "", std::string vl = "") : name(nm), value(vl) {}

	XmlNode* addchild(std::string nm = "", std::string vl = "");
};

class Xml {
public:
	static XmlNode* Parse(const std::string& path);
	static XmlNode FromVec(std::string nm, Vec2 v);
	static XmlNode FromVec(std::string nm, Vec3 v);
	static XmlNode FromVec(std::string nm, Vec4 v);

	static void ToVec(XmlNode* n, Vec2& v);
	static void ToVec(XmlNode* n, Vec3& v);
	static void ToVec(XmlNode* n, Vec4& v);

	static void Write(XmlNode* node, const std::string& path);
protected:
	static bool Read(std::string& s, uint& pos, XmlNode* parent);

	static void DoWrite(XmlNode* n, std::ofstream& strm, int ind);
};