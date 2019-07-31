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