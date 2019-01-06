#pragma once
#include "Engine.h"
#include <cstdint>
#include <cstdio>

#define _(s) (Localizer::useDict? Localizer::dict[HASH(s)] : s)
#define __(s) s

constexpr uint32_t HASH(const char* s) {
	unsigned long hash = 5381;
	int c = 0;
	while (c = *s++)
		hash = hash * 33 + c;
	return hash;
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