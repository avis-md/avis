#include "Engine.h"

Texture3D::Texture3D(uint x, uint y, uint z, byte chn, byte* data) : chn(chn), width(x), height(y), depth(z) {
	auto tp = (chn == 1) ? GL_RED : ((chn == 2) ? GL_RG : ((chn == 1) ? GL_RGB : GL_RGBA));
	glGenTextures(1, &pointer);
	glBindTexture(GL_TEXTURE_3D, pointer);
	glTexImage3D(GL_TEXTURE_3D, 0, tp, width, height, depth, 0, tp, GL_UNSIGNED_BYTE, data);
	SetTexParams<GL_TEXTURE_3D>();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_3D, 0);

	loaded = true;
}

void Texture3D::SetPixels(byte* data) {
	auto tp = (chn == 1) ? GL_RED : ((chn == 2) ? GL_RG : ((chn == 1) ? GL_RGB : GL_RGBA));
	glBindTexture(GL_TEXTURE_3D, pointer);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, tp, GL_UNSIGNED_BYTE, data);
}

void Texture3D::SetPixels(float* data) {
	auto tp = (chn == 1) ? GL_RED : ((chn == 2) ? GL_RG : ((chn == 1) ? GL_RGB : GL_RGBA));
	glBindTexture(GL_TEXTURE_3D, pointer);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, tp, GL_FLOAT, data);
}