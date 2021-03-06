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
#include "res/shd/coreVert.h"
#include "res/shd/coreFrag.h"
#include "res/shd/coreFrag3.h"

glm::mat3 UI::matrix;
bool UI::matrixIsI;

std::unordered_map<size_t, Vec2> UI::scrollWs;
Vec2* UI::currentScroll;
float UI::currentScrollW0;

bool UI::_isDrawingLoop = false;

UniqueCallerList UI::_editTextCallee;

Font UI::font, UI::font2;

bool UI::focused = true, UI::editingText = false;
uint UI::_editTextCursorPos = 0;
uint UI::_editTextCursorPos2 = 0;
std::string UI::_editTextString = "";
float UI::_editTextBlinkTime = 0;
float UI::_editTextHoff;

UI::Style UI::_defaultStyle = {};
float UI::alpha = 1;

uint UI::_vboSz = 32;
GLuint UI::_vao = 0;
GLuint UI::_vboV = 0;
GLuint UI::_vboU = 0;
GLuint UI::_tvbo = 0;

byte UI::_layer, UI::_layerMax;
bool UI::ignoreLayers = false;

Shader UI::quadProgC, UI::quadProgT;

Vec3 AU(Vec3 vec) {
	if (!UI::matrixIsI) {
		vec.y = Display::height - vec.y;
		vec = UI::matrix * vec;
		vec.y = Display::height - vec.y;
	}
	return vec;
}

void UI::Init() {
	_defaultStyle.fontSize = 12;
	_defaultStyle.normal.Set(white(1, 0.3f), black());
	_defaultStyle.mouseover.Set(white(), black());
	_defaultStyle.highlight.Set(blue(), white());
	_defaultStyle.press.Set(white(1, 0.5f), black());

	quadProgC = Shader::FromVF(glsl::coreVert, glsl::coreFrag3);
	quadProgT = Shader::FromVF(glsl::coreVert, glsl::coreFrag);

	quadProgC.AddUniform("col");
#define LC(nm) quadProgT.AddUniform(#nm)
	LC(sampler);
	LC(col);
	LC(level);
#undef LC

	InitVao();

	font = Font(IO::path + "res/font.ttf");
	font2 = Font(IO::path + "res/font2.ttf");
	if (!font) {
		Debug::Error("UI", "failed to open default font (/res/font.ttf)!");
		if (font2) font = font2;
		else Debug::Warning("UI", "failed to open alternate font (/res/font2.ttf)! Non-ascii text will not work!");
	}
	else if (!font2) font2 = font;

	ResetMatrix();
}

void UI::IncLayer() {
	_layer++;
}

void UI::PreLoop() {
	_editTextCallee.Preloop();

	editingText = _editTextCallee.last.frames[0];
	_layerMax = _layer;
	_layer = 0;
}

void UI::Rotate(float aa, Vec2 point) {
	float a = -aa * deg2rad;
	matrix *= glm::mat3(1, 0, 0, 0, 1, 0, point.x, point.y, 1)
		* glm::mat3(cos(a), -sin(a), 0, sin(a), cos(a), 0, 0, 0, 1)
		* glm::mat3(1, 0, 0, 0, 1, 0, -point.x, -point.y, 1);
	matrixIsI = false;
}
void UI::ResetMatrix() {
	matrix = glm::mat3(1.f);
	matrixIsI = true;
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
	for (int y = 0; y < 4; ++y) {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(UI::matrixIsI ? quadPoss[y] : UI::matrix*quadPoss[y]);
	}

	UI::SetVao(4, quadPoss);

	Engine::defProg.Bind();
	glUniform4f(Engine::defProg.Loc(0), col.r, col.g, col.b, col.a * UI::alpha);
	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Engine::quadBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void UI::Quad(float x, float y, float w, float h, GLuint tex, Vec4 col, int mip, Vec2 uv0, Vec2 uv1, Vec2 uv2, Vec2 uv3) {
	if (!tex || col.a <= 0) return;
	Vec3 quadPoss[4];
	quadPoss[0].x = x;		quadPoss[0].y = y;
	quadPoss[1].x = x + w;	quadPoss[1].y = y;
	quadPoss[2].x = x;		quadPoss[2].y = y + h;
	quadPoss[3].x = x + w;	quadPoss[3].y = y + h;
	for (int y = 0; y < 4; ++y) {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(UI::matrixIsI ? quadPoss[y] : UI::matrix*quadPoss[y]);
	}
	Vec2 quadUvs[4]{ uv0, uv1, uv2, uv3 };

	UI::SetVao(4, quadPoss, quadUvs);

	quadProgT.Bind();
	glUniform1i(quadProgT.Loc(0), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform4f(quadProgT.Loc(1), col.r, col.g, col.b, col.a * UI::alpha);
	glUniform1f(quadProgT.Loc(2), mip);
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
		UI::Quad(x, y, w, h, tex, tint, miplevel, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0));
	else if (scl == DRAWTEX_FIT) {
		float w2h = ((float)texture.width) / texture.height;
		if (w / h > w2h)
			UI::Quad(x + 0.5f*(w - h*w2h), y, h*w2h, h, tex, tint, miplevel, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0));
		else
			UI::Quad(x, y + 0.5f*(h - w / w2h), w, w / w2h, tex, tint, miplevel, Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0));
	}
	else if (scl == DRAWTEX_CROP) {
		float w2h = ((float)texture.width) / texture.height;
		if (w / h > w2h) {
			float dh = (1 - ((h * texture.width / w) / texture.height)) / 2;
			UI::Quad(x, y, w, h, tex, tint, miplevel, Vec2(0, 1-dh), Vec2(1, 1-dh), Vec2(0, dh), Vec2(1, dh));
		}
		else {
			float dw = (1 - ((w * texture.height / h) / texture.width)) / 2;
			UI::Quad(x, y, w, h, tex, tint, miplevel, Vec2(dw, 1), Vec2(1 - dw, 1), Vec2(dw, 0), Vec2(1 - dw, 0));
		}
	}
	else {
		UI::Quad(x, y, w, h, tex, tint, miplevel);
	}
}

std::string UI::EditText(float x, float y, float w, float h, float s, Vec4 bcol, const std::string& str2, bool delayed, Vec4 fcol, bool* changed, std::string str22, Font* font, Vec4 hcol, Vec4 acol, bool ser) {
	Engine::PushStencil(x, y, w, h);
	std::string str = str2;
	if (!str22.size()) str22 = str2;
	bool isActive = _editTextCallee.Add();

	if (changed) *changed = false;

	if (isActive) {
		auto al = font->alignment;
		font->Align(ALIGN_MIDLEFT);
		if (!delayed) _editTextString = str;
		if (!!_editTextCursorPos) _editTextCursorPos -= Input::KeyDown(KEY::LeftArrow);
		_editTextCursorPos += Input::KeyDown(KEY::RightArrow);
		_editTextCursorPos = Clamp<uint>(_editTextCursorPos, 0U, _editTextString.size());
		if (!Input::KeyHold(KEY::LeftShift) && (Input::KeyDown(KEY::LeftArrow) || Input::KeyDown(KEY::RightArrow))) {
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
		if (Input::KeyDown(KEY::Backspace)) {
			if (_editTextCursorPos == _editTextCursorPos2) {
				_editTextString = _editTextString.substr(0, _editTextCursorPos - 1) + _editTextString.substr(_editTextCursorPos);
				if (!!_editTextCursorPos) {
					_editTextCursorPos--;
					_editTextCursorPos2--;
				}
			}
			else {
				_editTextString = _editTextString.substr(0, std::min(_editTextCursorPos, _editTextCursorPos2)) + _editTextString.substr(std::max(_editTextCursorPos, _editTextCursorPos2));
				_editTextCursorPos2 = _editTextCursorPos = std::min(_editTextCursorPos, _editTextCursorPos2);
			}
			if (!delayed && changed) *changed = true;
			_editTextBlinkTime = 0;
		}
		if (Input::KeyDown(KEY::Delete) && _editTextCursorPos < _editTextString.size()) {
			_editTextString = _editTextString.substr(0, _editTextCursorPos) + _editTextString.substr(_editTextCursorPos + 1);
			if (!delayed && changed) *changed = true;
			_editTextBlinkTime = 0;
		}
		if (_editTextCursorPos != _editTextCursorPos2) {
			auto met = std::min(_editTextCursorPos, _editTextCursorPos2);
			auto mex = std::max(_editTextCursorPos, _editTextCursorPos2);
			if (Input::CheckCopy(&_editTextString[met], mex - met)) {
				_editTextString = _editTextString.substr(0, met) + _editTextString.substr(mex);
				_editTextCursorPos2 = _editTextCursorPos = met;
			}
		}
		UI::Quad(x, y, w, h, black());
		UI::Quad(x + 1, y + 1, w - 2, h - 2, white());
		UI::Label(x + 2 - _editTextHoff, y + 0.4f*h, s, _editTextString);

		auto szz = _editTextString.size();
		if (!!Input::mouse0State && !!szz && Rect(x, y, w, h).Inside(Input::mouseDownPos)) {
			_editTextCursorPos = 0;
			for (uint i = 1; i <= szz; ++i) {
				_editTextCursorPos += (Input::mousePos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
			}
			if (Input::mouse0State == MOUSE_DOWN) {
				_editTextCursorPos2 = 0;
				for (uint i = 1; i <= szz; ++i) {
					_editTextCursorPos2 += (Input::mouseDownPos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
				}
			}
			_editTextBlinkTime = 0;
		}

		float xp;
		if (!_editTextCursorPos) xp = x + 2 - _editTextHoff;
		else xp = font->poss[_editTextCursorPos * 4].x*Display::width;

		float xp2;
		if (!_editTextCursorPos2) xp2 = x + 2 - _editTextHoff;
		else xp2 = font->poss[_editTextCursorPos2 * 4].x*Display::width;
		if (_editTextCursorPos != _editTextCursorPos2) {
			UI::Quad(xp, y + 2, xp2 - xp, h - 4, hcol);
			UI::Label(std::min(xp, xp2), y + 0.4f*h, s, _editTextString.substr(std::min(_editTextCursorPos, _editTextCursorPos2), abs((int)_editTextCursorPos - (int)_editTextCursorPos2)), acol);
		}
		_editTextBlinkTime += Time::delta;
		if (fmod(_editTextBlinkTime, 1) < 0.5f) Engine::DrawLine(Vec2(xp + 2, y + 2), Vec2(xp + 2, y + h - 2), (_editTextCursorPos == _editTextCursorPos2) ? black() : white(), 1);
		font->Align(al);
		
		if (xp < x + 2) {
			_editTextHoff -= (x - xp + 2);
		}
		else if (xp > x + w - 2) {
			_editTextHoff += (xp - x - w + 2);
		}

		Engine::PopStencil();
		if ((Input::mouse0State == MOUSE_DOWN && !Rect(x, y, w, h).Inside(Input::mouseDownPos)) || Input::KeyDown(KEY::Enter)) {
			_editTextCallee.Clear();
			if (changed && delayed) *changed = true;

			return delayed ? _editTextString : str;
		}
		if (Input::KeyDown(KEY::Escape)) {
			_editTextCallee.Clear();
			return str;
		}
		return delayed ? str : _editTextString;
	}
	else if (Engine::Button(x, y, w, h, bcol, str22, s, fcol) == MOUSE_RELEASE) {
		_editTextCallee.Set();
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
	bool isActive = _editTextCallee.Add();

	if (changed) *changed = false;

	if (isActive) {
		auto al = font->alignment;
		font->Align(ALIGN_MIDLEFT);

		if (!delayed) _editTextString = str;
		if (!!_editTextCursorPos) _editTextCursorPos -= Input::KeyDown(KEY::LeftArrow);
		_editTextCursorPos += Input::KeyDown(KEY::RightArrow);
		_editTextCursorPos = Clamp<uint>(_editTextCursorPos, 0U, _editTextString.size());
		if (!Input::KeyHold(KEY::LeftShift) && (Input::KeyDown(KEY::LeftArrow) || Input::KeyDown(KEY::RightArrow))) {
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
		if (Input::KeyDown(KEY::Backspace)) {
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
		if (Input::KeyDown(KEY::Delete) && _editTextCursorPos < _editTextString.size()) {
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
			for (uint i = 1; i <= szz; ++i) {
				_editTextCursorPos += (Input::mousePos.x > Display::width*(font->poss[i * 4 - 2].x + font->poss[i * 4].x) / 2);
			}
			_editTextCursorPos2 = 0;
			for (uint i = 1; i <= szz; ++i) {
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

		if ((Input::mouse0State == MOUSE_UP && !Rect(x, y, w, h).Inside(Input::mousePos)) || Input::KeyDown(KEY::Enter)) {
			_editTextCallee.Clear();
			if (changed && delayed) *changed = true;
			return delayed ? _editTextString : str;
		}
		if (Input::KeyDown(KEY::Escape)) {
			_editTextCallee.Clear();
			return str;
		}
		return delayed ? str : _editTextString;
	}
	else {
		pstr.resize(str.size(), repl);
		if (Engine::Button(x, y, w, h, bcol, pstr, s, fcol, false) == MOUSE_RELEASE) {
			_editTextCallee.Set();
			_editTextCursorPos = str.size();
			_editTextCursorPos2 = 0;
			_editTextBlinkTime = 0;
			if (delayed) _editTextString = str;
		}
	}
	return str;
}

float UI::GetLabelW(float s, std::string str, Font* font) {
	if (!s || !str[0]) return 0;
	float totalW = 0;
	char* cc = &str[0];
	while(*cc) {
		auto c = Font::utf2unc(cc);
		if (c < 0x0100) totalW += font->params[(uint)s][0].o2s[c & 0x00ff];
		else totalW += font2.params[(uint)s][c & 0xff00].o2s[c & 0x00ff];
	}
	return totalW;
}

void UI::Label(float x, float y, float s, const std::string& st, Vec4 col, bool shad, float maxw, Font* font) {
	Label(x, y, s, &st[0], st.size(), col, shad, maxw, font);
}

void UI::Label(float x, float y, float s, const char* str, uint sz, Vec4 col, bool shad, float maxw, Font* font) {
	if (!s || !str[0]) return;
	x = std::round(x);
	y = std::round(y);
	uint si = (uint)std::round(s);
	font->SizeVec(sz);
	byte align = (byte)font->alignment;
	float totalW = 0;
	std::vector<uint> ucs(sz), mks;
	uint usz = 0;
	char* cc = (char*)str;
	for (uint a = 0; *cc > 0 && a < sz; a++) {
		auto mk = ucs[usz++] = Font::utf2unc(cc);
		mk &= 0xff00;
		if (std::find(mks.begin(), mks.end(), mk) == mks.end()) {
			mks.push_back(mk);
			if (!mk) font->glyph(si, 0);
			else font2.glyph(si, mk);
		}
	}
	auto& prm = font->params[(uint)s][0];
	auto& uprm2 = font2.params[(uint)s];
	for (uint i = 0; i < usz; ++i) {
		auto& c = ucs[i];
		if (c < 0x0100) totalW += prm.o2s[c & 0x00ff];
		else totalW += uprm2[c & 0xff00].o2s[c & 0x00ff];
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
		auto& p = (!m) ? prm : uprm2[m];

		Vec3 off = Vec3(p.off[cc].x, -p.off[cc].y, 0);

		if (c == '\n') {
			font->poss[i] = font->poss[i + 1] =
				font->poss[i + 2] = font->poss[i + 3] = Vec3(-1, -1, 0);
			font->cs[i] = font->cs[i + 1] = font->cs[i + 2] = font->cs[i + 3] = 0;
			x = defx;
			y = std::roundf(y - s * 1.3f);
		}
		else {
			font->poss[i] = AU(Vec3(x - 4, y - s - 4, 1) + off)*ds;
			font->poss[i + 1] = AU(Vec3(x + s + 4, y - s - 4, 1) + off)*ds;
			font->poss[i + 2] = AU(Vec3(x - 4, y + 4, 1) + off)*ds;
			font->poss[i + 3] = AU(Vec3(x + s + 4, y + 4, 1) + off)*ds;
			font->cs[i] = font->cs[i + 1] = font->cs[i + 2] = font->cs[i + 3] = c;
			x += p.o2s[cc];
			if (c == ' ' || c == '\t')
				x = std::roundf(x);
		}
	}
	font->poss[usz * 4] = Vec3(x, 0, 0)*ds;

	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, usz * 4 * sizeof(Vec3), &font->poss[0]);
	glBindBuffer(GL_ARRAY_BUFFER, Font::vbos[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, usz * 4 * sizeof(int), &font->cs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glActiveTexture(GL_TEXTURE0);
	
	if (shad) {
		for (auto& m : mks) {
			(!m) ? font->sglyph(si, 0) : font2.sglyph(si, m);
		}
		Font::prog.Bind();
		glUniform4f(Font::prog.Loc(0), 0, 0, 0, col.a * alpha);
		glUniform1i(Font::prog.Loc(1), 0);
		glUniform2f(Font::prog.Loc(2), 1.f / Display::width, -1.f / Display::height);
		glBindVertexArray(Font::vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Font::idbuf);
		for (auto& m : mks) {
			GLuint tex = (!m) ? font->sglyph(si, 0) : font2.sglyph(si, m);
			//Font::prog.Bind();
			glBindTexture(GL_TEXTURE_2D, tex);
			glUniform1i(Font::prog.Loc(3), m);

			glDrawElements(GL_TRIANGLES, 6 * usz, GL_UNSIGNED_INT, 0);
		}
	}

	for (auto& m : mks) {
		(!m) ? font->glyph(si, 0) : font2.glyph(si, m);
	}
	Font::prog.Bind();
	glUniform4f(Font::prog.Loc(0), col.r, col.g, col.b, col.a * alpha);
	glUniform2f(Font::prog.Loc(2), 0, 0);
	glUniform1i(Font::prog.Loc(1), 0);
	glBindVertexArray(Font::vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Font::idbuf);
	for (auto& m : mks) {
		GLuint tex = (!m) ? font->glyph(si, 0) : font2.glyph(si, m);
		//Font::prog.Bind();
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(Font::prog.Loc(3), m);

		glDrawElements(GL_TRIANGLES, 6 * usz, GL_UNSIGNED_INT, 0);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

float UI::BeginScroll(float x, float y, float w, float h, float off) {
	Engine::PushStencil(x, y, w, h);
	uintptr_t buf[3];
	Debug::StackTrace(3, (void**)buf);
	currentScroll = &scrollWs[buf[2]];
	if (off >= 0) currentScroll->y = off;
	if (Rect(x,y,w,h).Inside(Input::mousePos)) currentScroll->y += Input::mouseScroll * 10;
	if (currentScroll->x > h) currentScroll->y = Clamp(currentScroll->y, h - currentScroll->x, 0.f);
	else currentScroll->y = 0;
	currentScrollW0 = y + currentScroll->y;
	return currentScrollW0;
}

void UI::EndScroll(float off) {
	currentScroll->x = (off - currentScrollW0);
	Engine::EndStencil();
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