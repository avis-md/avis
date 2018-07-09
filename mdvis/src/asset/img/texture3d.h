#pragma once
#include "AssetObjects.h"

class Texture3D : public AssetObject {
public:
	Texture3D(const string& path, TEX_FILTERING filter = TEX_FILTER_TRILINEAR);

	bool loaded;
	uint length;
	GLuint pointer;
protected:
	Texture3D() : AssetObject(ASSETTYPE_TEXCUBE) {}
};
