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

class Texture {
public:
	Texture(const std::string& path, bool mipmap = true, TEX_FILTERING filter = TEX_FILTER_BILINEAR, byte aniso = 5, TEX_WRAPING wrap = TEX_WRAP_REPEAT);
	Texture(const std::string& path, bool mipmap, TEX_FILTERING filter, byte aniso, GLenum wrapx, GLenum wrapy);
	Texture(const byte* data, const uint dataSz, TEX_FILTERING filter = TEX_FILTER_BILINEAR, TEX_WRAPING wrap = TEX_WRAP_CLAMP);
	~Texture() { glDeleteTextures(1, &pointer); }
	bool loaded;
	uint width, height;
	GLuint pointer;
	TEX_TYPE texType() { return _texType; }

	bool tiled = false;
	Int2 tileSize = Int2(1, 1);
	float tileSpeed = 2;

	static byte* LoadPixels(const std::string& path, byte& chn, uint& w, uint& h);
	static byte* LoadPixels(const byte* data, const uint dataSz, uint& w, uint& h);

	static void ToPNG(std::vector<byte>& data, uint w, uint h, const std::string& loc);
	
	friend class RenderTexture;
	_allowshared(Texture);
protected:
	Texture() = default;
	static TEX_TYPE _ReadStrm(Texture* tex, std::istream& strm, byte& chn, GLenum& rgb, GLenum& rgba);
	byte _aniso = 0;
	TEX_FILTERING _filter = TEX_FILTER_POINT;
	TEX_TYPE _texType = TEX_TYPE_NORMAL;
	bool _mipmap = true, _repeat = false, _blurmips = false;

	void GenECache(byte* dat, byte chn, bool isrgb, std::vector<RenderTexture*>* rts);
};
