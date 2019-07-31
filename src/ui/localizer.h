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
#include <cstdint>
#include <cstdio>

#define _(s) (Localizer::useDict? Localizer::dict[HASH(s)] : s)
#define __(s) s

constexpr uint32_t _HASH(char* s, uint32_t v) {
	return (*s)? _HASH(s+1, v * 33 + *s) : v;
}

constexpr uint32_t HASH(const char* s) {
	return _HASH((char*)s, 5381);
}

class Localizer {
public:
    static void Init(const std::string& nm);

	static bool useDict;

    static void MakeMap(std::string path);
    static void _MakeMap(std::string path, std::map<uint32_t, std::string>& strs);

	static void Merge(std::string path, std::map<uint32_t, std::string> strs);

    static std::unordered_map<uint32_t, std::string> dict;
};