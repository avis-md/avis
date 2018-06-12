#pragma once
#include "Engine.h"

void _StreamWrite(const void* val, std::ofstream* stream, int size);

template<typename T> void _Strm2Val(std::istream& strm, T &val) {
	long long pos = strm.tellg();
	strm.read((char*)&val, (std::streamsize)sizeof(T));
	if (strm.fail()) {
		Debug::Error("Strm2Val", "Fail bit raised! (probably eof reached) " + std::to_string(pos));
	}
}