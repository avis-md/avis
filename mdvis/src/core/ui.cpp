#include "Engine.h"
#include "res/shddata.h"

bool UI::_isDrawingLoop = false;
uintptr_t UI::_activeEditText[UI_MAX_EDIT_TEXT_FRAMES] = {};
uintptr_t UI::_lastEditText[UI_MAX_EDIT_TEXT_FRAMES] = {};
uintptr_t UI::_editingEditText[UI_MAX_EDIT_TEXT_FRAMES] = {};
ushort UI::_activeEditTextId = 0;
ushort UI::_editingEditTextId = 0;

Font* UI::font, *UI::font2;

bool UI::focused = true, UI::editingText = false;
uint UI::_editTextCursorPos = 0;
uint UI::_editTextCursorPos2 = 0;
std::string UI::_editTextString = "";
float UI::_editTextBlinkTime = 0;

UI::Style UI::_defaultStyle = {};
float UI::alpha = 1;

uint UI::_vboSz = 32;
GLuint UI::_vao = 0;
GLuint UI::_vboV = 0;
GLuint UI::_vboU = 0;
GLuint UI::_tvbo = 0;

byte UI::_layer, UI::_layerMax;
bool UI::ignoreLayers = false;

PROGDEF(UI::quadProgC)
PROGDEF(UI::quadProgT)

void UI::Init() {
	_defaultStyle.fontSize = 12;
	_defaultStyle.normal.Set(white(1, 0.3f), black());
	_defaultStyle.mouseover.Set(white(), black());
	_defaultStyle.highlight.Set(blue(), white());
	_defaultStyle.press.Set(white(1, 0.5f), black());

	quadProgC = Shader::FromVF(glsl::coreVert, glsl::coreFrag3);
	quadProgT = Shader::FromVF(glsl::coreVert, glsl::coreFrag);

	quadProgCLocs[0] = glGetUniformLocation(quadProgC, "col");
#define LC(nm) quadProgTLocs[i++] = glGetUniformLocation(quadProgT, #nm)
	int i = 0;
	LC(sampler);
	LC(col);
	LC(level);
#undef LC

	InitVao();

	font = new Font(IO::path + "res/font.ttf");
	font2 = new Font(IO::path + "res/font2.ttf");
	if (!font) {
		Debug::Error("UI", "failed to open default font (/res/font.ttf)!");
		if (font2) font = font2;
		else Debug::Warning("UI", "failed to open alternate font (/res/font2.ttf)! Non-ascii text will not work!");
	}
	else if (!font2) font2 = font;
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
	glGenTextures(1, &_tvbo);
	glBindTexture(GL_TEXTURE_BUFFER, _tvbo);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, _vboV);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
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
	}
	glBindBuffer(GL_ARRAY_BUFFER, _vboV);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sz * sizeof(Vec3), verts);
	if (uvs) {
		glBindBuffer(GL_ARRAY_BUFFER, _vboU);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sz * sizeof(Vec2), uvs);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool UI::IsSameId(uintptr_t* left, uintptr_t* right) {
	for (byte a = 0; a < UI_MAX_EDIT_TEXT_FRAMES; ++a)  {
		if (left[a] != right[a]) return false;
	}
	return true;
}

void UI::GetEditTextId() {
	memset(_activeEditText, 0, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
	Debug::StackTrace(UI_MAX_EDIT_TEXT_FRAMES, (void**)_activeEditText);
	if (IsSameId(_activeEditText, _lastEditText)) _activeEditTextId++;
	else _activeEditTextId = 0;
	memcpy(_lastEditText, _activeEditText, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
}

void UI::PreLoop() {
	memset(_lastEditText, 0, UI_MAX_EDIT_TEXT_FRAMES * sizeof(uintptr_t));
	_activeEditTextId = 0;

	editingText = !!_editingEditText[0];
	_layerMax = _layer;
	_layer = 0;
}

void UI::Quad(float x, float y, float w, float h, Vec4 col) {
	if (col.a <= 0) return;
	Vec3 quadPoss[4];
	quadPoss[0].x = x;
	quadPoss[0].y = y;
	quadPoss[1].x = x + w;
	quadPoss[1].y = y;
	quadPoss[2].x = x;
	quadPoss[2].y = y + h;
	quadPoss[3].x = x + w;
	quadPoss[3].y = y + h;
	for (int y = 0; y < 4; ++y)  {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(Display::uiMatrix*quadPoss[y]);
	}

	UI::SetVao(4, quadPoss);

	glUseProgram(Engine::defProgram);
	glUniform4f(Engine::defColLoc, col.r, col.g, col.b, col.a * UI::alpha);
	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Engine::quadBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void UI::Quad(float x, float y, float w, float h, GLuint tex, Vec4 col, Vec2 uv0, Vec2 uv1, Vec2 uv2, Vec2 uv3) {
	if (!tex || col.a <= 0) return;
	Vec3 quadPoss[4];
	quadPoss[0].x = x;		quadPoss[0].y = y;
	quadPoss[1].x = x + w;	quadPoss[1].y = y;
	quadPoss[2].x = x;		quadPoss[2].y = y + h;
	quadPoss[3].x = x + w;	quadPoss[3].y = y + h;
	for (int y = 0; y < 4; ++y)  {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(Display::uiMatrix*quadPoss[y]);
	}
	Vec2 quadUvs[4]{ uv0, uv1, uv2, uv3 };

	UI::SetVao(4, quadPoss, quadUvs);

	glUseProgram(quadProgT);
	glUniform1i(quadProgTLocs[0], 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform4f(quadProgTLocs[1], col.r, col.g, col.b, col.a * UI::alpha);
	glUniform1f(quadProgTLocs[2], 0);
	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Engine::quadBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void UI::Texture(float x, float y, float w, float h, const ::Texture& texture, DRAWTEX_SCALING scl, float miplevel) {
	UI::Texture(x, y, w, h, texture, white(), scl, miplevel);
}
void UI::Texture(float x, float y, float w, float h, const ::Texture& texture, float alpha, DRAWTEX_SCALING scl, float miplevel) {
	UI::Texture(x, y, w, h, texture, white(alpha), scl, miplevel);
}
void UI::Texture(float x, float y, float w, float h, const ::Texture& texture, Vec4 tint, DRAWTEX_SCALING scl, float miplevel) {
	if (!texture) return;
	auto tex = texture.pointer;
	if (scl == DRAWTEX_STRETCH)
		UI::Quad(x, y, w, h, tex, tint, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0));
	else if (scl == DRAWTEX_FIT) {
		float w2h = ((float)texture.width) / texture.height;
		if (w / h > w2h)
			UI::Quad(x + 0.5f*(w - h*w2h), y, h*w2h, h, tex, tint, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0));
		else
			UI::Quad(x, y + 0.5f*(h - w / w2h), w, w / w2h, tex, tint, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0));
	}
	else if (scl == DRAWTEX_CROP) {
		float w2h = ((float)texture.width) / texture.height;
		if (w / h > w2h) {
			float dh = (1 - ((h * texture.width / w) / texture.height)) / 2;
			UI::Quad(x, y, w, h, tex, tint, Vec2(0, 1-dh), Vec2(1, 1-dh), Vec2(0, dh), Vec2(1, dh));
		}
		else {
			float dw = (1 - ((w * texture.height / h) / texture.width)) / 2;
			UI::Quad(x, y, w, h, tex, tint, Vec2(dw, 1), Vec2(1 - dw, 1), Vec2(dw, 0), Vec2(1 - dw, 0));
		}
	}
	else {
		UI::Quad(x, y, w, h, tex, tint);
	}
}

std::string UI::EditText(float x, float y, float w, float h, float s, Vec4 bcol, const std::string& str2, bool delayed, Vec4 fcol, bool* changed, std::string str22, Font* font, Vec4 hcol, Vec4 acol, bool ser) {
	Engine::PushStencil(x, y, w, h);
	std::string str = str2;
	if (!str22.size()) str22 = str2;
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
				_editTextString = _editTextString.substr(0, std::min(_editTextCursorPos, _editTextCursorPos2)) + Input::inputString + _editTextString.substr(std::max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos = std::min(_editTextCursorPos, _editTextCursorPos2) + ssz;
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
				_editTextString = _editTextString.substr(0, std::min(_editTextCursorPos, _editTextCursorPos2)) + _editTextString.substr(std::max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos = std::min(_editTextCursorPos, _editTextCursorPos2);
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
		UI::Quad(x, y, w, h, black());
		UI::Quad(x + 1, y + 1, w - 2, h - 2, white());
		UI::Label(x + 2, y + 0.4f*h, s, _editTextString);

		auto szz = _editTextString.size();
		if (!!Input::mouse0State && !!szz && Rect(x, y, w, h).Inside(Input::mousePos)) {
			_editTextCursorPos = 0;
			for (uint i = 1; i <= szz; ++i)  {
				_editTextCursorPos += (Input::mousePos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
			}
			_editTextCursorPos2 = 0;
			for (uint i = 1; i <= szz; ++i)  {
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
			UI::Quad(xp, y + 2, xp2 - xp, h - 4, hcol);
			UI::Label(std::min(xp, xp2), y + 0.4f*h, s, _editTextString.substr(std::min(_editTextCursorPos, _editTextCursorPos2), abs((int)_editTextCursorPos - (int)_editTextCursorPos2)), acol);
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
	else if (Engine::Button(x, y, w, h, bcol, str22, s, fcol) == MOUSE_RELEASE) {
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

std::string UI::EditTextPass(float x, float y, float w, float h, float s, Vec4 bcol, const std::string& str2, char repl, bool delayed, Vec4 fcol, bool* changed, Font* font, Vec4 hcol, Vec4 acol, bool ser) {
	std::string str = str2;
	std::string pstr = "";
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
				_editTextString = _editTextString.substr(0, std::min(_editTextCursorPos, _editTextCursorPos2)) + Input::inputString + _editTextString.substr(std::max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos = std::min(_editTextCursorPos, _editTextCursorPos2) + ssz;
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
				_editTextString = _editTextString.substr(0, std::min(_editTextCursorPos, _editTextCursorPos2)) + _editTextString.substr(std::max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos = std::min(_editTextCursorPos, _editTextCursorPos2);
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
		UI::Quad(x, y, w, h, black());
		UI::Quad(x + 1, y + 1, w - 2, h - 2, white());
		pstr.resize(_editTextString.size(), repl);
		UI::Label(x + 2, y + 0.4f*h, s, pstr, black());

		auto szz = _editTextString.size();
		if (!!Input::mouse0State && !!szz && Rect(x, y, w, h).Inside(Input::mousePos)) {
			_editTextCursorPos = 0;
			for (uint i = 1; i <= szz; ++i)  {
				_editTextCursorPos += (Input::mousePos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
			}
			_editTextCursorPos2 = 0;
			for (uint i = 1; i <= szz; ++i)  {
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
			UI::Quad(xp, y + 2, xp2 - xp, h - 4, hcol);
			pstr.resize(_editTextString.size(), repl);
			UI::Label(std::min(xp, xp2), y + 0.4f*h, s, pstr.substr(std::min(_editTextCursorPos, _editTextCursorPos2), abs((int)_editTextCursorPos - (int)_editTextCursorPos2)), acol);
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

float UI::GetLabelW(float s, std::string str, Font* font) {
	if (!s || !str[0]) return 0;
	uint si = (uint)std::roundf(s);
	size_t sz = str.size();
	float totalW = 0;
	char* cc = &str[0];
	while(cc) {
		auto c = Font::utf2unc(cc);
		if (c < 0x0100) totalW += font->params[(uint)s][0].o2s[c & 0x00ff];
		else totalW += font2->params[(uint)s][c & 0xff00].o2s[c & 0x00ff];
	}
	return totalW;
}

void UI::Label(float x, float y, float s, std::string st, Vec4 col, float maxw, Font* font) {
	Label(x, y, s, &st[0], st.size(), col, maxw, font);
}

void UI::Label(float x, float y, float s, const char* str, uint sz, Vec4 col, float maxw, Font* font) {
	if (!s || !str[0]) return;
	uint si = (uint)std::roundf(s);
	sz = std::min(sz, (uint)strlen(str));
	font->SizeVec(sz);
	byte align = (byte)font->alignment;
	float totalW = 0;
	std::vector<uint> ucs(sz), mks;
	uint usz = 0;
	char* cc = (char*)str;
	while (!!*cc) {
		auto mk = ucs[usz++] = Font::utf2unc(cc);
		mk &= 0xff00;
		if (std::find(mks.begin(), mks.end(), mk) == mks.end()) {
			mks.push_back(mk);
			if (!mk) font->glyph(si, 0);
			else font2->glyph(si, mk);
		}
	}
	for (uint i = 0; i < usz; ++i)  {
		auto& c = ucs[i];
		if (c < 0x0100) totalW += font->params[(uint)s][0].o2s[c & 0x00ff];
		else totalW += font2->params[(uint)s][c & 0xff00].o2s[c & 0x00ff];
		if (maxw > 0 && totalW > maxw) {
			usz = i;
			break;
		}
	}
	if ((align & 15) > 0) {
		x -= totalW * (align & 15) * 0.5f;
	}

	y -= (1 - (0.5f * (align >> 4))) * s;

	y = Display::height - y;
	Vec3 ds = Vec3(1.f / Display::width, 1.f / Display::height, 0.5f);
	x = std::roundf(x);
	float defx = x;
	for (uint i = 0; i < usz * 4; i += 4) {
		auto& c = ucs[i / 4];
		auto m = c & 0xff00;
		auto cc = c & 0x00ff;
		auto& prm = (!m) ? font->params[(uint)s][0] : font2->params[(uint)s][m];
		//if (c == '\n')
		//	c = ' ';

		Vec3 off = Vec3(prm.off[cc].x, -prm.off[cc].y, 0);

		if (c == '\n') {
			font->poss[i] = font->poss[i + 1] =
				font->poss[i + 2] = font->poss[i + 3] = Vec3(-1, -1, 0);
			font->cs[i] = font->cs[i + 1] = font->cs[i + 2] = font->cs[i + 3] = 0;
			x = defx;
			y = std::roundf(y - s * 1.3f);
		}
		else {
			font->poss[i] = AU(Vec3(x - 1, y - s - 1, 1) + off)*ds;
			font->poss[i + 1] = AU(Vec3(x + s + 1, y - s - 1, 1) + off)*ds;
			font->poss[i + 2] = AU(Vec3(x - 1, y + 1, 1) + off)*ds;
			font->poss[i + 3] = AU(Vec3(x + s + 1, y + 1, 1) + off)*ds;
			font->cs[i] = font->cs[i + 1] = font->cs[i + 2] = font->cs[i + 3] = c;
			x += prm.o2s[cc];
			if (c == ' ' || c == '\t')
				x = std::roundf(x);
		}
	}
	font->poss[usz * 4] = Vec3(x, 0, 0)*ds;

	if (usz * 4 > Font::vaoSz) {
		Font::InitVao(usz * 4);
	}
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, usz * 4 * sizeof(Vec3), &font->poss[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, usz * 4 * sizeof(Vec2), &font->uvs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, usz * 4 * sizeof(int), &font->cs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Font::idbuf);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, usz * 6 * sizeof(uint), &font->ids[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(Font::fontProgram);
	glUniform4f(Font::fontProgLocs[0], col.r, col.g, col.b, col.a * alpha);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(Font::vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Font::idbuf);

	glUniform1i(Font::fontProgLocs[1], 0);

	for (auto m : mks) {
		GLuint tex = (!m) ? font->glyph(si, 0) : font2->glyph(si, m);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(Font::fontProgLocs[2], m);

		glDrawElements(GL_TRIANGLES, 6 * usz, GL_UNSIGNED_INT, 0);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}