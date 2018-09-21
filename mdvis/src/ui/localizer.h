#include "Engine.h"
#include <stdint.h>
#include <cstdio>

#define _(s) (Localizer::useDict? Localizer::dict[HASH(s)] : s)

constexpr uint32_t HASH_H1(const char* s, const uint32_t i, const uint32_t x) {
    return (x*65599u+(uint8_t)s[(i)<(strlen_c(s)-1-(i))]);
}
constexpr uint32_t HASH_H4(const char* s, const uint32_t i, const uint32_t x) {
    return HASH_H1(s,i,HASH_H1(s,i+1,HASH_H1(s,i+2,HASH_H1(s,i+3,x))));
}
constexpr uint32_t HASH_H16(const char* s, const uint32_t i, const uint32_t x) {
    return HASH_H4(s,i,HASH_H4(s,i+4,HASH_H4(s,i+8,HASH_H4(s,i+12,x))));
}
constexpr uint32_t HASH_H64(const char* s, const uint32_t i, const uint32_t x) {
    return HASH_H16(s,i,HASH_H16(s,i+16,HASH_H16(s,i+32,HASH_H16(s,i+48,x))));
}
constexpr uint32_t HASH_H256(const char* s, const uint32_t i, const uint32_t x) {
    return HASH_H64(s,i,HASH_H64(s,i+64,HASH_H64(s,i+128,HASH_H64(s,i+192,x))));
}
constexpr uint32_t HASH(const char* s) {
    return (HASH_H256(s,0,0)^(HASH_H256(s,0,0)>>16));
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