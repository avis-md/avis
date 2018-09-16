#pragma once
#include "Engine.h"

#include <ft2build.h>
#include FT_FREETYPE_H

/*! Dynamic fonts constructed with FreeType (.ttf files)
[av]*/
class Font {
public:
	Font(const string& path, ALIGNMENT align = ALIGN_TOPLEFT);
	bool loaded = false;
	GLuint glyph(uint size, uint mask);

	ALIGNMENT alignment;

	Font* Align(ALIGNMENT a);

	static uint utf2unc(char*& c);

	static void Init();

	friend class UI;
protected:
	static FT_Library _ftlib;
	static void InitVao(uint sz);
	FT_Face _face;
	static GLuint fontProgram;
	static GLint fontProgLocs[5];

	struct _params {
		float o2s[256];
		Vec2 off[256];
	};

	std::unordered_map<uint, std::unordered_map<uint, _params>> params;

	uint vecSize;
	std::vector<Vec3> poss;
	std::vector<Vec2> uvs;
	std::vector<uint> ids;
	std::vector<uint> cs;
	void SizeVec(uint sz);

	static uint vaoSz;
	static GLuint vao, vbos[3], idbuf;

	std::unordered_map<uint, std::unordered_map<uint, GLuint>> _glyphs; //each glyph size is fontSize*16
	GLuint CreateGlyph(uint size, uint mask = 0);
};