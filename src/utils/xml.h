// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "Engine.h"

struct XmlNode {
	std::string name = "", value = "";
	std::unordered_map<std::string, std::string> params;
	std::vector<XmlNode> children;

	XmlNode(std::string nm = "", std::string vl = "") : name(nm), value(vl), params({}), children(0) {}

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