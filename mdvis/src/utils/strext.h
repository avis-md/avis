#pragma once
#include "Engine.h"

int constexpr strlen_c(const char* str)
{
	return *str ? 1 + strlen_c(str + 1) : 0;
}

namespace std {
	string to_string(Vec2 v);
	string to_string(Vec3 v);
	string to_string(Vec4 v);
	string to_string(Quat v);
}

std::vector<string> string_split(string s, char c, bool removeBlank = false);
int string_find(const string& s, const string& s2, int start = -1);
string to_lowercase(const string& s);
string rm_spaces(const string& s);

int TryParse(string str, int defVal);
uint TryParse(string str, uint defVal);
float TryParse(string str, float defVal);
double TryParse(string str, double defVal);