#include "Engine.h"

bool UI::_isDrawingLoop = false;
uintptr_t UI::_activeEditText[UI_MAX_EDIT_TEXT_FRAMES] = {};
uintptr_t UI::_lastEditText[UI_MAX_EDIT_TEXT_FRAMES] = {};
uintptr_t UI::_editingEditText[UI_MAX_EDIT_TEXT_FRAMES] = {};
ushort UI::_activeEditTextId = 0;
ushort UI::_editingEditTextId = 0;
Font* UI::font;
bool UI::focused = true, UI::editingText = false;
uint UI::_editTextCursorPos = 0;
uint UI::_editTextCursorPos2 = 0;
string UI::_editTextString = "";
float UI::_editTextBlinkTime = 0;
UI::Style UI::_defaultStyle = {};
float UI::alpha = 1;

uint UI::_vboSz = 32;
GLuint UI::_vao = 0;
GLuint UI::_vboV = 0;
GLuint UI::_vboU = 0;

byte UI::_layer, UI::_layerMax;

void UI::Init() {
	_defaultStyle.fontSize = 12;
	_defaultStyle.normal.Set(white(1, 0.3f), black());
	_defaultStyle.mouseover.Set(white(), black());
	_defaultStyle.highlight.Set(blue(), white());
	_defaultStyle.press.Set(white(1, 0.5f), black());

	InitVao();

	font = new Font(IO::path + "/res/font.ttf");
}

void UI::InitVao() {
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vboV);
	glGenBuffers(1, &_vboU);
	glBindBuffer(GL_ARRAY_BUFFER, _vboV);
	glBufferData(GL_ARRAY_BUFFER, _vboSz * sizeof(Vec3), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, _vboU);
	glBufferData(GL_ARRAY_BUFFER, _vboSz * sizeof(Vec2), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(_vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, _vboV);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, _vboU);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void UI::IncLayer() {
	_layer++;
}

void UI::SetVao(uint sz, void* verts, void* uvs) {
	assert(!!sz && verts);
	if (sz > _vboSz) {
		glDeleteBuffers(1, &_vboV);
		glDeleteBuffers(1, &_vboU);
		glDeleteVertexArrays(1, &_vao);
		_vboSz = sz;
		InitVao();
#ifdef IS_EDITOR
		PopupSelector::InitVao();
#endif
	}
	//auto e = glGetError();
	glBindBuffer(GL_ARRAY_BUFFER, _vboV);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sz * sizeof(Vec3), verts);
	if (uvs) {
		glBindBuffer(GL_ARRAY_BUFFER, _vboU);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sz * sizeof(Vec2), uvs);
	}
	//e = glGetError();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool UI::IsSameId(uintptr_t* left, uintptr_t* right) {
	for (byte a = 0; a < UI_MAX_EDIT_TEXT_FRAMES; a++) {
		if (left[a] != right[a]) return false;
	}
	return true;
}

void UI::GetEditTextId() {
	//#ifdef PLATFORM_WIN
	memset(_activeEditText, 0, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
	Debug::StackTrace(UI_MAX_EDIT_TEXT_FRAMES, (void**)_activeEditText);
	//UI_Trace(drawFuncLoc, 2, _activeEditText);
	if (IsSameId(_activeEditText, _lastEditText)) _activeEditTextId++;
	else _activeEditTextId = 0;

	memcpy(_lastEditText, _activeEditText, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
	//#endif
}

bool UI::IsActiveEditText() {
	return false;
}

void UI::PreLoop() {
	memset(_lastEditText, 0, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
	_activeEditTextId = 0;

	editingText = !!_editingEditText[0];
	_layerMax = _layer;
	_layer = 0;
}

#define _checkdraw assert(UI::CanDraw() && "UI functions can only be called from the Overlay function!");

bool UI::CanDraw() {
	return (std::this_thread::get_id() == Engine::_mainThreadId);
}

void UI::Texture(float x, float y, float w, float h, ::Texture* texture, DRAWTEX_SCALING scl, float miplevel) {
	UI::Texture(x, y, w, h, texture, white(), scl, miplevel);
}
void UI::Texture(float x, float y, float w, float h, ::Texture* texture, float alpha, DRAWTEX_SCALING scl, float miplevel) {
	UI::Texture(x, y, w, h, texture, white(alpha), scl, miplevel);
}
void UI::Texture(float x, float y, float w, float h, ::Texture* texture, Vec4 tint, DRAWTEX_SCALING scl, float miplevel) {
	_checkdraw;
	GLuint tex = (texture->loaded) ? texture->pointer : Engine::fallbackTex->pointer;
	if (!texture->tiled) {
		if (scl == DRAWTEX_STRETCH)
			Engine::DrawQuad(x, y, w, h, tex, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0), false, tint, miplevel);
		else if (scl == DRAWTEX_FIT) {
			float w2h = ((float)texture->width) / texture->height;
			if (w / h > w2h)
				Engine::DrawQuad(x + 0.5f*(w - h*w2h), y, h*w2h, h, tex, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0), false, tint, miplevel);
			else
				Engine::DrawQuad(x, y + 0.5f*(h - w / w2h), w, w / w2h, tex, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0), false, tint, miplevel);
		}
		else if (scl == DRAWTEX_CROP) {
			float w2h = ((float)texture->width) / texture->height;
			if (w / h > w2h) {
				float dh = (1 - ((h * texture->width / w) / texture->height)) / 2;
				Engine::DrawQuad(x, y, w, h, tex, Vec2(0, 1-dh), Vec2(1, 1-dh), Vec2(0, dh), Vec2(1, dh), false, tint, miplevel);
			}
			else {
				float dw = (1 - ((w * texture->height / h) / texture->width)) / 2;
				Engine::DrawQuad(x, y, w, h, tex, Vec2(dw, 1), Vec2(1 - dw, 1), Vec2(dw, 0), Vec2(1 - dw, 0), false, tint, miplevel);
			}
		}
		else {
			Engine::DrawQuad(x, y, w, h, tex, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0), false, tint, miplevel);
		}
	}
	else {
		int ix = (int)floor(Repeat<float>(Time::time * texture->tileSpeed, 0.0f, (float)texture->tileSize.x));
		int iy = (int)floor(Repeat<float>(Time::time * texture->tileSpeed / texture->tileSize.x, 0.0f, (float)texture->tileSize.y));
		ix = min(ix, texture->tileSize.x - 1);
		iy = min(iy, texture->tileSize.y - 1);
		const float dx = 1.0f / texture->tileSize.x;
		const float dy = 1.0f / texture->tileSize.y;
		const float x1 = dx * ix;
		const float x2 = dx * (ix + 1);
		const float y1 = 1 - dy * (iy + 1);
		const float y2 = 1 - dy * (iy);
		Engine::DrawQuad(x, y, w, h, tex, Vec2(x1, y2), Vec2(x2, y2), Vec2(x1, y1), Vec2(x2, y1), false, tint, miplevel);
	}
}

string UI::EditText(float x, float y, float w, float h, float s, Vec4 bcol, const string& str2, bool delayed, Vec4 fcol, bool* changed, Font* font, Vec4 hcol, Vec4 acol, bool ser) {
	Engine::PushStencil(x, y, w, h);
	string str = str2;
	_checkdraw;
	GetEditTextId();
	bool isActive = (UI::IsSameId(_activeEditText, _editingEditText) && (_activeEditTextId == _editingEditTextId));

	if (changed) *changed = false;

	if (isActive) {
		auto al = font->alignment;
		font->Align(ALIGN_MIDLEFT);
		if (!delayed) _editTextString = str;
		if (!!_editTextCursorPos) _editTextCursorPos -= Input::KeyDown(Key_LeftArrow);
		_editTextCursorPos += Input::KeyDown(Key_RightArrow);
		_editTextCursorPos = Clamp<uint>(_editTextCursorPos, 0U, _editTextString.size());
		if (!Input::KeyHold(Key_LeftShift) && (Input::KeyDown(Key_LeftArrow) || Input::KeyDown(Key_RightArrow))) {
			_editTextCursorPos2 = _editTextCursorPos;
			_editTextBlinkTime = 0;
		}
		auto ssz = Input::inputString.size();
		if (ssz) {
			if (_editTextCursorPos == _editTextCursorPos2) {
				_editTextString = _editTextString.substr(0, _editTextCursorPos) + Input::inputString + _editTextString.substr(_editTextCursorPos);
				_editTextCursorPos += ssz;
				_editTextCursorPos2 += ssz;
			}
			else {
				_editTextString = _editTextString.substr(0, min(_editTextCursorPos, _editTextCursorPos2)) + Input::inputString + _editTextString.substr(max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos = min(_editTextCursorPos, _editTextCursorPos2) + ssz;
				_editTextCursorPos2 = _editTextCursorPos;
			}
			if (!delayed && changed) *changed = true;
			_editTextBlinkTime = 0;
		}
		if (Input::KeyDown(Key_Backspace)) {
			if (_editTextCursorPos == _editTextCursorPos2) {
				_editTextString = _editTextString.substr(0, _editTextCursorPos - 1) + _editTextString.substr(_editTextCursorPos);
				if (!!_editTextCursorPos) {
					_editTextCursorPos--;
					_editTextCursorPos2--;
				}
			}
			else {
				_editTextString = _editTextString.substr(0, min(_editTextCursorPos, _editTextCursorPos2)) + _editTextString.substr(max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos = min(_editTextCursorPos, _editTextCursorPos2);
				_editTextCursorPos2 = _editTextCursorPos;
			}
			if (!delayed && changed) *changed = true;
			_editTextBlinkTime = 0;
		}
		if (Input::KeyDown(Key_Delete) && _editTextCursorPos < _editTextString.size()) {
			_editTextString = _editTextString.substr(0, _editTextCursorPos) + _editTextString.substr(_editTextCursorPos + 1);
			if (!delayed && changed) *changed = true;
			_editTextBlinkTime = 0;
		}
		Engine::DrawQuad(x, y, w, h, black());
		Engine::DrawQuad(x + 1, y + 1, w - 2, h - 2, white());
		UI::Label(x + 2, y + 0.4f*h, s, _editTextString);

		auto szz = _editTextString.size();
		if (!!Input::mouse0State && !!szz && Rect(x, y, w, h).Inside(Input::mousePos)) {
			_editTextCursorPos = 0;
			for (uint i = 1; i <= szz; i++) {
				_editTextCursorPos += (Input::mousePos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
			}
			_editTextCursorPos2 = 0;
			for (uint i = 1; i <= szz; i++) {
				_editTextCursorPos2 += (Input::mouseDownPos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
			}
			_editTextBlinkTime = 0;
		}

		float xp;
		if (!_editTextCursorPos) xp = x + 2;
		else xp = font->poss[_editTextCursorPos * 4].x*Display::width;
		float xp2;
		if (!_editTextCursorPos2) xp2 = x + 2;
		else xp2 = font->poss[_editTextCursorPos2 * 4].x*Display::width;
		if (_editTextCursorPos != _editTextCursorPos2) {
			Engine::DrawQuad(xp, y + 2, xp2 - xp, h - 4, hcol);
			UI::Label(min(xp, xp2), y + 0.4f*h, s, _editTextString.substr(min(_editTextCursorPos, _editTextCursorPos2), abs((int)_editTextCursorPos - (int)_editTextCursorPos2)), acol);
		}
		_editTextBlinkTime += Time::delta;
		if (fmod(_editTextBlinkTime, 1) < 0.5f) Engine::DrawLine(Vec2(xp, y + 2), Vec2(xp, y + h - 2), (_editTextCursorPos == _editTextCursorPos2) ? black() : white(), 1);
		font->Align(al);

		Engine::PopStencil();
		if ((Input::mouse0State == MOUSE_UP && !Rect(x, y, w, h).Inside(Input::mousePos)) || Input::KeyDown(Key_Enter)) {
			memset(_editingEditText, 0, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
			_activeEditTextId = 0;
			if (changed && delayed) *changed = true;

			return delayed ? _editTextString : str;
		}
		if (Input::KeyDown(Key_Escape)) {
			memset(_editingEditText, 0, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
			_activeEditTextId = 0;
			return str;
		}
		return delayed ? str : _editTextString;
	}
	else if (Engine::Button(x, y, w, h, bcol, str, s, fcol) == MOUSE_RELEASE) {
		memcpy(_editingEditText, _activeEditText, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
		_editingEditTextId = _activeEditTextId;
		_editTextCursorPos = str.size();
		_editTextCursorPos2 = 0;
		_editTextBlinkTime = 0;
		if (delayed) _editTextString = str;
	}
	Engine::PopStencil();
	return str;
	//#endif
}

string UI::EditTextPass(float x, float y, float w, float h, float s, Vec4 bcol, const string& str2, char repl, bool delayed, Vec4 fcol, bool* changed, Font* font, Vec4 hcol, Vec4 acol, bool ser) {
	string str = str2;
	string pstr = "";
	_checkdraw;
	GetEditTextId();
	bool isActive = (UI::IsSameId(_activeEditText, _editingEditText) && (_activeEditTextId == _editingEditTextId));

	if (changed) *changed = false;

	if (isActive) {
		auto al = font->alignment;
		font->Align(ALIGN_MIDLEFT);

		if (!delayed) _editTextString = str;
		if (!!_editTextCursorPos) _editTextCursorPos -= Input::KeyDown(Key_LeftArrow);
		_editTextCursorPos += Input::KeyDown(Key_RightArrow);
		_editTextCursorPos = Clamp<uint>(_editTextCursorPos, 0U, _editTextString.size());
		if (!Input::KeyHold(Key_LeftShift) && (Input::KeyDown(Key_LeftArrow) || Input::KeyDown(Key_RightArrow))) {
			_editTextCursorPos2 = _editTextCursorPos;
			_editTextBlinkTime = 0;
		}
		auto ssz = Input::inputString.size();
		if (ssz) {
			if (_editTextCursorPos == _editTextCursorPos2) {
				_editTextString = _editTextString.substr(0, _editTextCursorPos) + Input::inputString + _editTextString.substr(_editTextCursorPos);
				_editTextCursorPos += ssz;
				_editTextCursorPos2 += ssz;
			}
			else {
				_editTextString = _editTextString.substr(0, min(_editTextCursorPos, _editTextCursorPos2)) + Input::inputString + _editTextString.substr(max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos = min(_editTextCursorPos, _editTextCursorPos2) + ssz;
				_editTextCursorPos2 = _editTextCursorPos;
			}
			if (!delayed && changed) *changed = true;
			_editTextBlinkTime = 0;
		}
		if (Input::KeyDown(Key_Backspace)) {
			if (_editTextCursorPos == _editTextCursorPos2) {
				_editTextString = _editTextString.substr(0, _editTextCursorPos - 1) + _editTextString.substr(_editTextCursorPos);
				if (!!_editTextCursorPos) {
					_editTextCursorPos--;
					_editTextCursorPos2--;
				}
			}
			else {
				_editTextString = _editTextString.substr(0, min(_editTextCursorPos, _editTextCursorPos2)) + _editTextString.substr(max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos = min(_editTextCursorPos, _editTextCursorPos2);
				_editTextCursorPos2 = _editTextCursorPos;
			}
			if (!delayed && changed) *changed = true;
			_editTextBlinkTime = 0;
		}
		if (Input::KeyDown(Key_Delete) && _editTextCursorPos < _editTextString.size()) {
			_editTextString = _editTextString.substr(0, _editTextCursorPos) + _editTextString.substr(_editTextCursorPos + 1);
			if (!delayed && changed) *changed = true;
			_editTextBlinkTime = 0;
		}
		Engine::DrawQuad(x, y, w, h, black());
		Engine::DrawQuad(x + 1, y + 1, w - 2, h - 2, white());
		pstr.resize(_editTextString.size(), repl);
		UI::Label(x + 2, y + 0.4f*h, s, pstr, black());

		auto szz = _editTextString.size();
		if (!!Input::mouse0State && !!szz && Rect(x, y, w, h).Inside(Input::mousePos)) {
			_editTextCursorPos = 0;
			for (uint i = 1; i <= szz; i++) {
				_editTextCursorPos += (Input::mousePos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
			}
			_editTextCursorPos2 = 0;
			for (uint i = 1; i <= szz; i++) {
				_editTextCursorPos2 += (Input::mouseDownPos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
			}
			_editTextBlinkTime = 0;
		}
		
		float xp;
		if (!_editTextCursorPos) xp = x + 2;
		else xp = font->poss[_editTextCursorPos * 4].x*Display::width;
		float xp2;
		if (!_editTextCursorPos2) xp2 = x + 2;
		else xp2 = font->poss[_editTextCursorPos2 * 4].x*Display::width;
		if (_editTextCursorPos != _editTextCursorPos2) {
			Engine::DrawQuad(xp, y + 2, xp2 - xp, h - 4, hcol);
			pstr.resize(_editTextString.size(), repl);
			UI::Label(min(xp, xp2), y + 0.4f*h, s, pstr.substr(min(_editTextCursorPos, _editTextCursorPos2), abs((int)_editTextCursorPos - (int)_editTextCursorPos2)), acol);
		}
		_editTextBlinkTime += Time::delta;
		if (fmod(_editTextBlinkTime, 1) < 0.5f) Engine::DrawLine(Vec2(xp, y + 2), Vec2(xp, y + h - 2), (_editTextCursorPos == _editTextCursorPos2) ? black() : white(), 1);
		font->Align(al);

		if ((Input::mouse0State == MOUSE_UP && !Rect(x, y, w, h).Inside(Input::mousePos)) || Input::KeyDown(Key_Enter)) {
			memset(_editingEditText, 0, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
			_activeEditTextId = 0;
			if (changed && delayed) *changed = true;
			return delayed ? _editTextString : str;
		}
		if (Input::KeyDown(Key_Escape)) {
			memset(_editingEditText, 0, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
			_activeEditTextId = 0;
			return str;
		}
		return delayed ? str : _editTextString;
	}
	else {
		pstr.resize(str.size(), repl);
		if (Engine::Button(x, y, w, h, bcol, pstr, s, fcol, false) == MOUSE_RELEASE) {
			memcpy(_editingEditText, _activeEditText, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
			_editingEditTextId = _activeEditTextId;
			_editTextCursorPos = str.size();
			_editTextCursorPos2 = 0;
			_editTextBlinkTime = 0;
			if (delayed) _editTextString = str;
		}
	}
	return str;
}
Vec3 AU(Vec3 vec) {
	if (!Display::uiMatrixIsI) {
		vec.y = Display::height - vec.y;
		vec = Display::uiMatrix * vec;
		vec.y = Display::height - vec.y;
	}
	return vec;
}

void UI::Label(float x, float y, float s, string st, Vec4 col, float maxw, Font* font) {
	Label(x, y, s, &st[0], st.size(), col, maxw, font);
}

void UI::Label(float x, float y, float s, const char* str, uint sz, Vec4 col, float maxw, Font* font) {
	sz = min(sz, (uint)strlen(str));
	if (s <= 0) return;
	GLuint tex = font->glyph((uint)round(s));
	font->SizeVec(sz);
	byte align = (byte)font->alignment;
	float totalW = 0;
	for (uint i = 0; i < sz; i ++) {
		auto& c = str[i];
		totalW += font->o2s[c] * s;
		if (maxw > 0 && totalW > maxw) {
			sz = i;
			break;
		}
	}
	if ((align & 15) > 0) {
		x -= totalW * (align & 15) * 0.5f;
	}

	y -= (1 - (0.5f * (align >> 4))) * s;

	y = Display::height - y;
	float w = 0;
	Vec3 ds = Vec3(1.0f / Display::width, 1.0f / Display::height, 0.5f);
	x = round(x);
	float defx = x;
	for (uint i = 0; i < sz * 4; i += 4) {
		auto& c = str[i / 4];
		//if (c == '\n')
		//	c = ' ';

		Vec3 off = -Vec3(font->off[c].x, font->off[c].y, 0)*s;
		w = font->w2h[c] * s;
		font->poss[i] = AU(Vec3(x, y - s, 1) + off)*ds;
		font->poss[i + 1] = AU(Vec3(x + s, y - s, 1) + off)*ds;
		font->poss[i + 2] = AU(Vec3(x, y, 1) + off)*ds;
		font->poss[i + 3] = AU(Vec3(x + s, y, 1) + off)*ds;
		font->cs[i] = c;
		font->cs[i + 1] = c;
		font->cs[i + 2] = c;
		font->cs[i + 3] = c;

		if (c == '\n') {
			x = defx;
			y -= s * 1.3f;
		}
		else {
			x += font->o2s[c] * s;
		}
	}
	font->poss[sz * 4] = Vec3(x, 0, 0)*ds;

	if (sz * 4 > Font::vaoSz) {
		Font::InitVao(sz * 4);
#ifdef IS_EDITOR
		PopupSelector::InitVaoF();
#endif
	}
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sz * 4 * sizeof(Vec3), &font->poss[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sz * 4 * sizeof(Vec2), &font->uvs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sz * 4 * sizeof(float), &font->cs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Font::idbuf);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sz * 6 * sizeof(uint), &font->ids[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(font->fontProgram);
	GLint baseColLoc = glGetUniformLocation(font->fontProgram, "col");
	glUniform4f(baseColLoc, col.r, col.g, col.b, col.a * alpha);
	GLint baseImageLoc = glGetUniformLocation(font->fontProgram, "sampler");
	glUniform1i(baseImageLoc, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(Font::vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Font::idbuf);
	glDrawElements(GL_TRIANGLES, 6 * sz, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}