#include "Engine.h"
#include "res/shddata.h"

FT_Library Font::_ftlib = nullptr;
GLuint Font::fontProgram = 0;
GLint Font::fontProgLocs[] = {};
uint Font::vaoSz = 0;
GLuint Font::vao = 0;
GLuint Font::vbos[] = { 0, 0, 0 };
GLuint Font::idbuf = 0;

void Font::Init() {
	int err = FT_Init_FreeType(&_ftlib);
	if (err != FT_Err_Ok) {
		Debug::Error("Font", "Fatal: Initializing freetype failed!");
	}

	fontProgram = Shader::FromVF(glsl::fontVert, glsl::fontFrag);
#define LC(nm) fontProgLocs[i++] = glGetUniformLocation(fontProgram, #nm)
	int i = 0;
	LC(col);
	LC(sampler);
	LC(mask);
#undef LC

	InitVao(500);
}

void Font::InitVao(uint sz) {
	vaoSz = sz;
	if (!!vao) {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(3, vbos);
		glDeleteBuffers(1, &idbuf); 
	}
	glGenVertexArrays(1, &vao);
	glGenBuffers(3, vbos);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, vaoSz * sizeof(Vec3), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, vaoSz * sizeof(Vec2), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glBufferData(GL_ARRAY_BUFFER, vaoSz * sizeof(int), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
	glVertexAttribIPointer(2, 1, GL_INT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glGenBuffers(1, &idbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz * 6 * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Font::Font(const string& path, ALIGNMENT align) : vecSize(0), alignment(align) {
	auto err = FT_New_Face(_ftlib, path.c_str(), 0, &_face);
	if (err != FT_Err_Ok) {
		Debug::Warning("Font", "Failed to load font! " + std::to_string(err));
		return;
	}
	//FT_Set_Char_Size(_face, 0, (FT_F26Dot6)(size * 64.0f), Display::dpi, 0); // set pixel size based on dpi
	FT_Set_Pixel_Sizes(_face, 0, 12); // set pixel size directly
	FT_Select_Charmap(_face, FT_ENCODING_UNICODE);
	CreateGlyph(12, 0);
	SizeVec(20);
	loaded = true;
}

GLuint Font::glyph(uint size, uint mask) {
	if (_glyphs.count(size) == 1) {
		auto& gly = _glyphs[size];
		if (gly.count(mask) == 1)
			return _glyphs[size][mask];
	}
	return CreateGlyph(size, mask);
}

void Font::SizeVec(uint sz) {
	if (vecSize >= sz) return;
	poss.resize(sz * 4 + 1);
	cs.resize(sz * 4);
	uvs.resize(sz * 4);
	ids.resize(sz * 6);
	for (; vecSize < sz; vecSize++) {
		uvs[vecSize * 4] = Vec2(0, 1);
		uvs[vecSize * 4 + 1] = Vec2(1, 1);
		uvs[vecSize * 4 + 2] = Vec2(0, 0);
		uvs[vecSize * 4 + 3] = Vec2(1, 0);
		ids[vecSize * 6] = 4 * vecSize;
		ids[vecSize * 6 + 1] = 4 * vecSize + 1;
		ids[vecSize * 6 + 2] = 4 * vecSize + 2;
		ids[vecSize * 6 + 3] = 4 * vecSize + 1;
		ids[vecSize * 6 + 4] = 4 * vecSize + 3;
		ids[vecSize * 6 + 5] = 4 * vecSize + 2;
	}
}

GLuint Font::CreateGlyph(uint sz, uint mask) {
	FT_Set_Pixel_Sizes(_face, 0, sz); // set pixel size directly
	_glyphs.emplace(sz, 0);
	glGenTextures(1, &_glyphs[sz][mask]);
	glBindTexture(GL_TEXTURE_2D, _glyphs[sz][mask]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, (sz + 2) * 16, (sz + 2) * 16);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	bool recalc = (params.count(mask) == 0);
	auto& pr = params[mask];
	for (ushort a = 0; a < 256; a++) {
		if (recalc) {
			pr.w2h[a] = 0;
			pr.o2s[a] = 0;
		}
		if (FT_Load_Char(_face, mask + a, FT_LOAD_RENDER) != FT_Err_Ok) continue;
		byte x = a % 16, y = a / 16;
		FT_Bitmap bmp = _face->glyph->bitmap;
		glTexSubImage2D(GL_TEXTURE_2D, 0, (sz + 2) * x + 1, (sz + 2) * y + 1, bmp.width, bmp.rows, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);
		if (recalc) {
			if (bmp.width == 0) {
				pr.w2h[a] = 0;
				pr.o2s[a] = 0;
			}
			else {
				pr.w2h[a] = (float)(bmp.width) / bmp.rows;
				pr.o2s[a] = _face->glyph->metrics.horiAdvance / sz / 64.0f;
			}
			pr.off[a] = Vec2((float)(_face->glyph->bitmap_left) / sz, 1 - ((float)(_face->glyph->bitmap_top) / sz));
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	if (recalc) {
		pr.o2s[' '] = 0.3f;
		pr.o2s['\t'] = 0.9f;
	}
	return _glyphs[sz][mask];
}

Font* Font::Align(ALIGNMENT a) {
	alignment = a;
	return this;
}

uint Font::utf2unc(char*& c) {
#define MK(cc) uint(*(cc) & 63)
	if (!((*c >> 7) & 1)) {
		return *(c++);
	}
	else if (!((*c >> 5) & 1)) {
		uint res = *c & 31;
		res = (res << 6) + MK(c+1);
		c += 2;
		return res;
	}
	else if (!((*c >> 4) & 1)) {
		uint res = *c & 15;
		res = (res << 12) + (MK(c + 1) << 6) + MK(c + 2);
		c += 3;
		return res;
	}
	else {
		uint res = *c & 7;
		res = (res << 18) + (MK(c + 1) << 12) + (MK(c + 2) << 6) + MK(c + 3);
		c += 4;
		return res;
	}
}