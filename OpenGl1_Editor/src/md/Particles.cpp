#include "Particles.h"


void Particles::Init() {

	byte data[256] = {};
	data[0] = data[4] = data[8] = 255;
	data[1] = data[2] = data[3] = data[5] = data[6] = data[7] = 85;
	glGenTextures(1, &colorPalleteTex);
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	uint* i = new uint[5000 * 1250];
	for (uint a = 0; a < 5000 * 1250; a++) {
		i[a] = 1 << 16 + 2 << 8 + 1;
	}

	glGenTextures(1, &colorIndexTex);
	glBindTexture(GL_TEXTURE_2D, colorIndexTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 5000, 5000, 0, GL_RGBA, GL_UNSIGNED_BYTE, i);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[](i);
}