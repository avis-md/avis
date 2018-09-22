#include "xml.h"

void RemoveNewLine(std::string& s) {
	for (int a = s.size() - 1; a > 0; a--) {
		if (s[a] == '\t' || s[a] == '\r' || s[a] == '\n') {
			s.replace(a, 1, " ");
		}
	}
}

std::string rm_st_ed_spaces(const std::string& s) {
	auto l = s.find_first_not_of(' ');
	if (l == std::string::npos) return "";
	return s.substr(l, s.find_last_not_of(' ') - l + 1);
}

bool NodeHasChild(std::string& s, int first) {
	for (int a = first; a < s.size(); a++) {
		if (s[a] == '<') return true;
		else if (s[a] > 0x20) return false;
	}
	return false;
}

XmlNode* Xml::Parse(const std::string& path) {
	auto str = IO::GetText(path);
	RemoveNewLine(str);
	
	XmlNode* node = new XmlNode();
	uint pos = 0, sz = str.size();
	while (pos < sz) {
		if (!Read(str, pos, node)) {
			delete(node);
			return nullptr;
		}
	}
	return node;
}

bool Xml::Read(std::string& s, uint& pos, XmlNode* parent) {
	size_t off, off2;
	off = s.find_first_of('<', pos);
	off2 = s.find_first_of('>', off);
	if (off2 < off) return false;
	if (off == std::string::npos) {
		pos = s.size();
		return true;
	}
	pos = off2 + 1;
	if (s[off + 1] == '?' && s[off2 - 1] == '?') {
		return true;
	}

	XmlNode n = {};
	auto ss = string_split(s.substr(off + 1, off2 - off - 1), ' ');
	if (!ss.size()) return false;
	n.name = ss[0];
	
	std::string ps = "";
	for (uint a = 1; a < ss.size(); a++) {
		if (ss[a] == "") continue;
		auto ss2 = string_split(ss[a], '=');
		ps = ss2[0];
		n.params.emplace(ss2[0], ss2[1].substr(1, ss2[1].size() - 2));
	}

	if (s[off2-1] == '/') {
		auto& lp = n.params[ps];
		lp = lp.substr(0, lp.size()-1);
	}
	else {
		auto ep = string_find(s, "</" + n.name + ">", off2);
		if (ep == -1) return false;
		if (!NodeHasChild(s, off2 + 1)) {
			n.value = rm_st_ed_spaces(s.substr(off2 + 1, ep - off2 - 1));
		}
		else {
			while (pos < (uint)ep) {
				Read(s, pos, &n);
			}
		}
		pos = ep + n.name.size() + 2;
	}
	parent->children.push_back(n);
	return true;
}

#define SVV(nm, vl) nd.addchild(nm, std::to_string(vl))
XmlNode Xml::FromVec(std::string nm, Vec2 v) {
	XmlNode nd(nm);
	SVV("x", v.x);
	SVV("y", v.y);
	return nd;
}
XmlNode Xml::FromVec(std::string nm, Vec3 v) {
	XmlNode nd(nm);
	SVV("x", v.x);
	SVV("y", v.y);
	SVV("z", v.y);
	return nd;
}
XmlNode Xml::FromVec(std::string nm, Vec4 v) {
	XmlNode nd(nm);
	SVV("x", v.x);
	SVV("y", v.y);
	SVV("z", v.z);
	SVV("w", v.w);
	return nd;
}

#define GT(nm, vl) if (n.name == #nm) vl = TryParse(n.value, 0.0f)
void Xml::ToVec(XmlNode* nd, Vec2& v) {
	for (auto& n : nd->children) {
		GT(x, v.x);
		else GT(y, v.y);
	}
}
void Xml::ToVec(XmlNode* nd, Vec3& v) {
	for (auto& n : nd->children) {
		GT(x, v.x);
		else GT(y, v.y);
		else GT(z, v.z);
	}
}
void Xml::ToVec(XmlNode* nd, Vec4& v) {
	for (auto& n : nd->children) {
		GT(x, v.x);
		else GT(y, v.y);
		else GT(z, v.z);
		else GT(w, v.w);
	}
}

XmlNode* XmlNode::addchild(std::string nm, std::string vl) {
	children.push_back(XmlNode(nm, vl));
	return &children.back();
}

void Xml::Write(XmlNode* node, const std::string& path) {
	std::ofstream strm(path);
	DoWrite(node, strm, 0);
}

void Xml::DoWrite(XmlNode* n, std::ofstream& strm, int ind) {
	for (int i = 0; i < ind; i++) strm << "    ";
	strm << "<" << n->name;
	if (n->params.size() > 0) {
		for (auto& p : n->params) {
			strm << " " << p.first << "=\"" << p.second << "\"";
		}
	}
	if (n->children.size() > 0) {
		strm << ">\n";
		for (auto& c : n->children) {
			DoWrite(&c, strm, ind+1);
		}
		for (int i = 0; i < ind; i++) strm << "    ";
		strm << "</" << n->name << ">\n";
	}
	else if (n->value != "") {
		if (n->value.size() > 10) {
			strm << ">\n";
			for (int i = 0; i <= ind; i++) strm << "    ";
			strm << n->value << "\n";
			for (int i = 0; i < ind; i++) strm << "    ";
		}
		else strm << ">" << n->value;
		strm << "</" << n->name << ">\n";
	}
	else strm << "/>\n";
}