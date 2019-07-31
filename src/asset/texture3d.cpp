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