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