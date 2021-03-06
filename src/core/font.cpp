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
#include "utils/effects.h"
#include "res/shd/minVert.h"
#include "res/shd/fontVert.h"
#include "res/shd/fontFrag.h"
#include "res/shd/fontBlurFrag.h"

FT_Library Font::_ftlib = nullptr;
Shader Font::prog, Font::blurProg;
uint Font::vaoSz = 0;
GLuint Font::vao = 0;
GLuint Font::vbos[] = { 0, 0 };
GLuint Font::idbuf = 0;

bool Font::waitkill = false;
int Font::facecnt = 0;

void Font::Init() {
	int err = FT_Init_FreeType(&_ftlib);
	if (err != FT_Err_Ok) {
		Debug::Error("Font", "Fatal: Initializing freetype failed!");
		abort();
	}

	(prog = Shader::FromVF(glsl::fontVert, glsl::fontFrag))
		.AddUniforms({ "col", "sampler", "off", "mask" });
	(blurProg = Shader::FromVF(glsl::minVert, glsl::fontBlurFrag))
		.AddUniforms({ "tex", "size", "rad", "isY" });
	InitVao(128);
	Unloader::Reg(Deinit);
}

void Font::Deinit() {
	if (!facecnt) {
		FT_Done_FreeType(_ftlib);
	}
	else waitkill = true;
}

void Font::InitVao(uint sz) {
	vaoSz = sz;
	if (!!vao) {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(2, vbos);
		glDeleteBuffers(1, &idbuf); 
	}
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbos);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, vaoSz * sizeof(Vec3), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, vaoSz * sizeof(int), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glVertexAttribIPointer(1, 1, GL_INT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glGenBuffers(1, &idbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz * 6 * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Font::Font(const std::string& path, ALIGNMENT align) : alignment(align), vecSize(0) {
	assert(!waitkill);
	auto err = FT_New_Face(_ftlib, path.c_str(), 0, &_face);
	if (err != FT_Err_Ok) {
		Debug::Warning("Font", "Failed to load font! " + std::to_string(err));
		return;
	}
	//FT_Set_Char_Size(_face, 0, (FT_F26Dot6)(size * 64.f), Display::dpi, 0); // set pixel size based on dpi
	FT_Set_Pixel_Sizes(_face, 0, 12); // set pixel size directly
	FT_Select_Charmap(_face, FT_ENCODING_UNICODE);
	CreateGlyph(12, 0);
	SizeVec(20);
	loaded = true;
	dpi = Display::dpiScl;
	facecnt++;
}

Font::~Font() {
	CheckUniqueRef();
}

void Font::DestroyRef() {
	if (loaded) {
		FT_Done_Face(_face);
		facecnt--;
		if (waitkill && !facecnt) {
			FT_Done_FreeType(_ftlib);
		}
	}
}

GLuint Font::glyph(uint size, uint mask) {
	if (dpi != Display::dpiScl) {
		params.clear();
		_glyphs.clear();
		_glyphShads.clear();
		dpi = Display::dpiScl;
	}
	auto it = _glyphs.find(size);
	if (it != _glyphs.end()) {
		auto& gly = it->second;
		auto it2 = gly.find(mask);
		if (it2 != gly.end())
			return it2->second;
	}
	return CreateGlyph(size, mask);
}

GLuint Font::sglyph(uint size, uint mask) {
	volatile GLuint _ = glyph(size, mask);
	if (dpi != Display::dpiScl) {
		params.clear();
		_glyphs.clear();
		_glyphShads.clear();
		dpi = Display::dpiScl;
	}
	auto it = _glyphShads.find(size);
	if (it != _glyphShads.end()) {
		auto& gly = it->second;
		auto it2 = gly.find(mask);
		if (it2 != gly.end())
			return it2->second;
	}
	return CreateSGlyph(size, mask);
}

void Font::ClearGlyphs() {
	_glyphs.clear();
}

void Font::SizeVec(uint sz) {
	if (vecSize >= sz) return;
	poss.resize(sz * 4 + 1);
	cs.resize(sz * 4);
	ids.resize(sz * 6);
	for (; vecSize < sz; ++vecSize) {
		ids[vecSize * 6] = 4 * vecSize;
		ids[vecSize * 6 + 1] = 4 * vecSize + 1;
		ids[vecSize * 6 + 2] = 4 * vecSize + 2;
		ids[vecSize * 6 + 3] = 4 * vecSize + 1;
		ids[vecSize * 6 + 4] = 4 * vecSize + 3;
		ids[vecSize * 6 + 5] = 4 * vecSize + 2;
	}

	vaoSz = sz * 4;
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sz * 4 * sizeof(Vec3), 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sz * 4 * sizeof(int), 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz * 6 * sizeof(uint), &ids[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLuint Font::CreateGlyph(uint sz, uint mask) {
	uint szd = (uint)(sz * Display::dpiScl);
	FT_Set_Pixel_Sizes(_face, 0, szd);
	_glyphs.emplace(sz, 0);
	glGenTextures(1, &_glyphs[sz][mask]);
	glBindTexture(GL_TEXTURE_2D, _glyphs[sz][mask]);
	std::vector<byte> _(std::pow((szd + 8) * 16, 2));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (szd + 8) * 16, (szd + 8) * 16, 0, GL_RED, GL_UNSIGNED_BYTE, _.data());
	SetTexParams<>();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	bool recalc = (params.count(sz) == 0) || (params[sz].count(mask) == 0);
	auto& pr = params[sz][mask];
	for (ushort a = 0; a < 256; ++a) {
		if (FT_Load_Char(_face, mask + a, FT_LOAD_RENDER) != FT_Err_Ok) continue;
		byte x = a % 16, y = a / 16;
		FT_Bitmap bmp = _face->glyph->bitmap;
		glTexSubImage2D(GL_TEXTURE_2D, 0, (szd + 8) * x + 4, (szd + 8) * y + 4, bmp.width, bmp.rows, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);
		if (recalc) {
			if (bmp.width == 0) {
				pr.o2s[a] = 0;
			}
			else {
				pr.o2s[a] = (float)(_face->glyph->advance.x >> 6) / Display::dpiScl;
			}
			pr.off[a] = Vec2(_face->glyph->bitmap_left, szd - _face->glyph->bitmap_top) / Display::dpiScl;
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	if (recalc && !mask) {
		pr.o2s[(uint)' '] = sz * 0.3f;
		pr.o2s[(uint)'\t'] = sz * 0.9f;
	}
	return _glyphs[sz][mask];
}

GLuint Font::CreateSGlyph(uint sz, uint mask) {
	uint szd = (uint)(sz * Display::dpiScl);
	uint sz2 = (szd + 8) * 16;
	GLuint texs[2];
	GLuint fbos[2];
	glGenTextures(2, texs);
	glGenFramebuffers(2, fbos);
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	for (int a = 0; a < 2; a++) {
		glBindTexture(GL_TEXTURE_2D, texs[a]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, sz2, sz2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		SetTexParams<>();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindFramebuffer(GL_FRAMEBUFFER, fbos[a]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texs[a], 0);
		glDrawBuffers(1, DrawBuffers);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			Debug::Error("Font::CreateSGlyph", "FB error:" + std::to_string(status));
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	blurProg.Bind();
	glUniform1i(blurProg.Loc(0), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _glyphs[sz][mask]);
	glUniform1i(blurProg.Loc(1), (int)sz2);
	glUniform1i(blurProg.Loc(2), 4);
	glUniform1i(blurProg.Loc(3), 0);
	
	glViewport(0, 0, sz2, sz2);
	glBindVertexArray(Camera::emptyVao);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindTexture(GL_TEXTURE_2D, texs[0]);
	glUniform1i(blurProg.Loc(3), 1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[1]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, Display::frameWidth, Display::frameHeight);
	blurProg.Unbind();

	glDeleteFramebuffers(2, fbos);
	glDeleteTextures(1, &texs[0]);
	_glyphShads[sz][mask] = texs[1];
	return texs[1];
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