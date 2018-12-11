#include "Engine.h"

namespace std {
	std::string to_string(Vec2 v) {
		return "(" + to_string(v.x) + ", " + to_string(v.y) + ")";
	}
	std::string to_string(Vec3 v) {
		return "(" + to_string(v.x) + ", " + to_string(v.y) + ", " + to_string(v.z) + ")";
	}
	std::string to_string(glm::dvec3 v) {
		return "(" + to_string(v.x) + ", " + to_string(v.y) + ", " + to_string(v.z) + ")";
	}
	std::string to_string(Vec4 v) {
		return "(" + to_string(v.x) + ", " + to_string(v.y) + ", " + to_string(v.z) + ", " + to_string(v.w) + ")";
	}
	std::string to_string(Quat v) {
		return "(" + to_string(v.x) + ", " + to_string(v.y) + ", " + to_string(v.z) + ", " + to_string(v.w) + ")";
	}
}

std::vector<std::string> string_split(std::string s, char c, bool rm) {
	std::vector<std::string> o = {};
	size_t pos = -1;
	do {
		s = s.substr(pos + 1);
		pos = s.find_first_of(c);
		if (!rm || pos > 0)
			o.push_back(s.substr(0, pos));
	} while (pos != std::string::npos);
	return o;
}

int string_find(const std::string& s, const std::string& s2, int start) {
	uint s2s = s2.size();
	int p = start;
	for (;;) {
		p = s.find_first_of(s2[0], p + 1);
		if (p == -1) return -1;
		if (s.substr(p, s2s) == s2) return p;
	}
	return -1;
}

std::string to_uppercase(std::string s) {
	for (auto& c : s) {
		if (c >= 'a' && c <= 'z')
			c += 'A' - 'a';
	}
	return s;
}

std::string to_lowercase(std::string s) {
	for (auto& c : s) {
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
	}
	return s;
}

std::string rm_spaces(const std::string& s) {
	std::string ss;
	ss.reserve(s.size());
	for (auto c : s) {
		if (c != ' ')
			ss += c;
	}
	return ss;
}

int TryParse(std::string str, int defVal) {
	try {
		return std::stoi(str);
	}
	catch (...) {
		return defVal;
	}
}
uint TryParse(std::string str, uint defVal) {
	try {
		if (str[0] == '-') return 0;
		else return std::stoul(str);
	}
	catch (...) {
		return defVal;
	}
}
float TryParse(std::string str, float defVal) {
	try {
		return std::stof(str);
	}
	catch (...) {
		return defVal;
	}
}
double TryParse(std::string str, double defVal) {
	try {
		return std::stod(str);
	}
	catch (...) {
		return defVal;
	}
}