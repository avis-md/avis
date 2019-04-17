#pragma once
#include "Engine.h"

class Texture3D {
public:
	Texture3D(uint x, uint y, uint z, byte chn, byte* data = 0);

	void SetPixels(byte* data), SetPixels(float* data);

	bool loaded;
	byte chn;
	uint width, height, depth;
	GLuint pointer;
};
