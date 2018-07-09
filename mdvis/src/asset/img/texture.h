#pragma once
#include "AssetObjects.h"

class Texture : public AssetObject {
public:
	Texture(const string& path, bool mipmap = true, TEX_FILTERING filter = TEX_FILTER_BILINEAR, byte aniso = 5, TEX_WRAPING wrap = TEX_WRAP_REPEAT);
	Texture(const string& path, bool mipmap, TEX_FILTERING filter, byte aniso, GLenum wrapx, GLenum wrapy);
	~Texture() { glDeleteTextures(1, &pointer); }
	bool loaded;
	uint width, height;
	GLuint pointer;
	TEX_TYPE texType() { return _texType; }

	bool tiled = false;
	Int2 tileSize = Int2(1, 1);
	float tileSpeed = 2;

	static byte* LoadPixels(const string& path, byte& chn, uint& w, uint& h);

	friend class AssetManager;
	friend class RenderTexture;
	_allowshared(Texture);
protected:
	Texture() : AssetObject(ASSETTYPE_TEXTURE) {}
	Texture(std::istream& strm, uint offset = 0);
	Texture(byte* b);
	static TEX_TYPE _ReadStrm(Texture* tex, std::istream& strm, byte& chn, GLenum& rgb, GLenum& rgba);
	byte _aniso = 0;
	TEX_FILTERING _filter = TEX_FILTER_POINT;
	TEX_TYPE _texType = TEX_TYPE_NORMAL;
	bool _mipmap = true, _repeat = false, _blurmips = false;
	bool DrawPreview(uint x, uint y, uint w, uint h) override;

	void GenECache(byte* dat, byte chn, bool isrgb, std::vector<RenderTexture*>* rts);
};
