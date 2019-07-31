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

int constexpr strlen_c(const char* str)
{
	return *str ? 1 + strlen_c(str + 1) : 0;
}

namespace std {
	std::string to_string(Vec2 v);
	std::string to_string(Vec3 v);
	std::string to_string(glm::dvec3 v);
	std::string to_string(Vec4 v);
	std::string to_string(Quat v);
}

std::vector<std::string> string_split(std::string s, char c, bool removeBlank = false);
int string_find(const std::string& s, const std::string& s2, int start = -1);
void string_triml(std::string& s);
void string_trimr(std::string& s);
std::string string_trim(std::string s);

std::string to_uppercase(std::string s);
std::string to_lowercase(std::string s);
std::string rm_spaces(const std::string& s);

int TryParse(std::string str, int defVal);
uint TryParse(std::string str, uint defVal);
float TryParse(std::string str, float defVal);
double TryParse(std::string str, double defVal);
