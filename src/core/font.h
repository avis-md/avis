#pragma once
#include "Engine.h"
#include "utils/refcnt.h"

#include <ft2build.h>
#include FT_FREETYPE_H

/*! Dynamic fonts constructed with FreeType (.ttf files)
[av]*/
class Font : public RefCnt {
public:
	Font() : loaded(false), _face(0) {}
	Font(const std::string& path, ALIGNMENT align = ALIGN_TOPLEFT);
	~Font();

	operator bool() const { return loaded; }

	static void Init(), Deinit();

	bool loaded = false;
	GLuint glyph(uint size, uint mask);
	GLuint sglyph(uint size, uint mask);
	void ClearGlyphs();

	float dpi;
	bool shadow;
	ALIGNMENT alignment;

	Font* Align(ALIGNMENT a);

	static uint utf2unc(char*& c);

	friend class UI;
protected:
	static FT_Library _ftlib;
	static void InitVao(uint sz);
	FT_Face _face;
	static Shader prog, blurProg;

	struct _params {
		float o2s[256];
		Vec2 off[256];
	};

	std::unordered_map<uint, std::unordered_map<uint, _params>> params;

	uint vecSize;
	std::vector<Vec3> poss;
	std::vector<uint> ids;
	std::vector<uint> cs;
	void SizeVec(uint sz);

	static uint vaoSz;
	static GLuint vao, vbos[2], idbuf;

	std::unordered_map<uint, std::unordered_map<uint, GLuint>>
		_glyphs, //each glyph size is fontSize*16
		_glyphShads; //shadow size is (fontSize + 6)*16
	GLuint CreateGlyph(uint size, uint mask = 0);
	GLuint CreateSGlyph(uint size, uint mask = 0);

	void DestroyRef() override;
	
	static bool waitkill;
	static int facecnt;
};