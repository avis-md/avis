#include <iostream>
#include "Engine.h"

const unsigned char pxls[] = { 255,0,0,255, 0,255,0,255, 0,0,255,255, 255,255,255,255 };

int main(int argc, char **argv) {
	byte chn;
	uint w, h;
	unsigned char* data = Texture::LoadPixels("tex_pixels.png", chn, w, h);
	assert(chn == 4 && w == 2 && h == 2);
	for (uint a = 0; a < 8; a++) {
		assert(pxls[a] == data[a]);
	}
	return 0;
}