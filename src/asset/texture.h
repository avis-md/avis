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

#pragma once
#include "AssetObjects.h"

template<GLenum T = GL_TEXTURE_2D>
void SetTexParams(int mp = 0, GLenum ws = GL_CLAMP_TO_EDGE, GLenum wt = GL_CLAMP_TO_EDGE, GLenum mn = GL_LINEAR, GLenum mg = GL_LINEAR) {
	glTexParameteri(T, GL_TEXTURE_MAX_LEVEL, mp);
	glTexParameteri(T, GL_TEXTURE_WRAP_S, ws);
	glTexParameteri(T, GL_TEXTURE_WRAP_T, wt);
	glTexParameteri(T, GL_TEXTURE_MIN_FILTER, mn);
	glTexParameteri(T, GL_TEXTURE_MAG_FILTER, mg);
}

class Texture : public RefCnt {
public:
	Texture() : loaded(false) {}
	Texture(std::nullptr_t) : loaded(false) {}
	Texture(const std::string& path, bool mipmap = true, TEX_FILTERING filter = TEX_FILTER_BILINEAR, byte aniso = 5, TEX_WRAPING wrap = TEX_WRAP_REPEAT);
	Texture(const std::string& path, bool mipmap, TEX_FILTERING filter, byte aniso, GLenum wrapx, GLenum wrapy);
	Texture(const byte* data, const uint dataSz, TEX_FILTERING filter = TEX_FILTER_BILINEAR, TEX_WRAPING wrap = TEX_WRAP_CLAMP);
	~Texture();

	operator bool() const {
		return loaded;
	}

	uint width, height;
	GLuint pointer = 0;

	void DestroyRef() override { glDeleteTextures(1, &pointer); }

	static byte* LoadPixels(const std::string& path, byte& chn, uint& w, uint& h);
	static byte* LoadPixels(const byte* data, const uint dataSz, uint& w, uint& h);

	static void ToPNG(std::vector<byte>& data, uint w, uint h, const std::string& loc);

private:
	bool loaded;

	static void InvertPNG(std::vector<byte>& data, uint x, uint y);

	static bool LoadJPEG(std::string fileN, uint &x, uint &y, byte& channels, byte** data);
	static bool LoadPNG(std::string fileN, uint &x, uint &y, byte& channels, std::vector<byte>& data);
	static bool LoadBMP(std::string fileN, uint &x, uint &y, byte& channels, byte** data);
};