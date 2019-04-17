#include "Engine.h"

void _StreamWrite(const void* val, std::ofstream* stream, int size) {
	stream->write(reinterpret_cast<char const *>(val), size);
}