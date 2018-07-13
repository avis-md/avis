#include "avi.h"
#include "aviriff2.h"

void AVI::Encode(const string& path, byte** frames, byte frameC, int w, int h, byte chn) {
	std::ofstream strm(path, std::ios::binary);
}