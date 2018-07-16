#include "Engine.h"
#include "res/shddata.h"

FT_Library Font::_ftlib = nullptr;
GLuint Font::fontProgram = 0;
uint Font::vaoSz = 0;
GLuint Font::vao = 0;
GLuint Font::vbos[] = { 0, 0, 0 };
GLuint Font::idbuf = 0;

void Font::Init() {
	int err = FT_Init_FreeType(&_ftlib);
	if (err != FT_Err_Ok) {
		Debug::Error("Font", "Fatal: Initializing freetype failed!");
		std::runtime_error("Fatal: Initializing freetype failed!");
	}

	fontProgram = Shader::FromVF(glsl::fontVert, glsl::fontFrag);

	InitVao(500);
}

void Font::InitVao(uint sz) {
#ifdef IS_EDITOR
	if (PopupSelector::drawing)
		glfwMakeContextCurrent(PopupSelector::mainWindow);
#endif
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
	glBufferData(GL_ARRAY_BUFFER, vaoSz * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
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
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glGenBuffers(1, &idbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz * 6 * sizeof(uint), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#ifdef IS_EDITOR
	if (PopupSelector::drawing)
		glfwMakeContextCurrent(PopupSelector::window);
#endif
}

Font::Font(const string& path, ALIGNMENT align) : vecSize(0), alignment(align) {
	Debug::Message("Font", "opening font at " + path);
	auto err = FT_New_Face(_ftlib, path.c_str(), 0, &_face);
	if (err != FT_Err_Ok) {
		Debug::Warning("Font", "Failed to load font! " + std::to_string(err));
		return;
	}
	//FT_Set_Char_Size(_face, 0, (FT_F26Dot6)(size * 64.0f), Display::dpi, 0); // set pixel size based on dpi
	FT_Set_Pixel_Sizes(_face, 0, 12); // set pixel size directly
	FT_Select_Charmap(_face, FT_ENCODING_UNICODE);
	CreateGlyph(12, true);
	SizeVec(20);
	loaded = true;
}

void Font::SizeVec(uint sz) {
	if (vecSize >= sz) return;
	poss.resize(sz * 4 + 1);
	cs.resize(sz * 4);
	for (; vecSize < sz; vecSize++) {
		uvs.push_back(Vec2(0, 1));
		uvs.push_back(Vec2(1, 1));
		uvs.push_back(Vec2(0, 0));
		uvs.push_back(Vec2(1, 0));
		ids.push_back(4 * vecSize);
		ids.push_back(4 * vecSize + 1);
		ids.push_back(4 * vecSize + 2);
		ids.push_back(4 * vecSize + 1);
		ids.push_back(4 * vecSize + 3);
		ids.push_back(4 * vecSize + 2);
	}
}

GLuint Font::CreateGlyph(uint sz, bool recalc) {
	FT_Set_Pixel_Sizes(_face, 0, sz); // set pixel size directly
	_glyphs.emplace(sz, 0);
	glGenTextures(1, &_glyphs[sz]);
	glBindTexture(GL_TEXTURE_2D, _glyphs[sz]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, (sz + 1) * 16, (sz + 1) * 16);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (ushort a = 0; a < 256; a++) {
		if (recalc) {
			w2h[a] = 0;
			o2s[a] = 0;
		}
		if (FT_Load_Char(_face, a, FT_LOAD_RENDER) != FT_Err_Ok) continue;
		byte x = a % 16, y = a / 16;
		FT_Bitmap bmp = _face->glyph->bitmap;
		glTexSubImage2D(GL_TEXTURE_2D, 0, (sz + 1) * x + 1, (sz + 1) * y, bmp.width, bmp.rows, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);
		if (recalc) {
			if (bmp.width == 0) {
				w2h[a] = 0;
				o2s[a] = 0;
			}
			else {
				w2h[a] = (float)(bmp.width) / bmp.rows;
				o2s[a] = _face->glyph->metrics.horiAdvance / sz / 64.0f;
			}
			off[a] = Vec2((float)(_face->glyph->bitmap_left) / sz, 1 - ((float)(_face->glyph->bitmap_top) / sz));
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	if (recalc) {
		o2s[' '] = 0.3f;
		o2s['\t'] = 0.9f;
	}
	return _glyphs[sz];
}

Font* Font::Align(ALIGNMENT a) {
	alignment = a;
	return this;
}